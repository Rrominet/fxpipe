#include "./Versions.h"
#include "./FxPipe.h"
#include "files.2/files.h"

#include <boost/process.hpp>

ml::Ret<> Versions::_initDir()
{
    if (FxPipe::get()->currentProject().empty())
        return ml::ret::fail("Stats::_initDir() : The project needs to be saved first.");
    auto d = files::parent(FxPipe::get()->currentProject());
    if (files::isDir(d + files::sep() + "." + files::name(FxPipe::get()->currentProject()) + "__versions"))
    {
        _directory = d + files::sep() + "." + files::name(FxPipe::get()->currentProject()) + "__versions";
        return ml::ret::success();
    }

    if (!files::exists(d))
        return ml::ret::fail("Stats::_initDir() : " + d + " does not exist.");
    if (!files::isDir(d))
        return ml::ret::fail("Stats::_initDir() : " + d + " is not a directory.");

    if (!files::mkdir(d + files::sep() + "." + files::name(FxPipe::get()->currentProject()) + "__versions"))
        return ml::ret::fail("Stats::_initDir() : Could not create " + d + files::sep() + "." + files::name(FxPipe::get()->currentProject()) + "__versions");
    _directory = d + files::sep() + "." + files::name(FxPipe::get()->currentProject()) + "__versions";
    return ml::ret::success();
}

void Versions::read(int quantity)
{
    _initDir();	
    _versions.clear();
    auto ls = files::ls(_directory, files::NAME_INV);
    if (ls.size() < quantity)
        quantity = ls.size();
    for (int i = 0; i < quantity; i++)
    {
        Version v;
        v.deserialize(json::parse(files::read(ls[i])));
        _versions.push_back(v);
    }
}

json Versions::serialize() const
{
    json _r = json::array();	 
    for (const auto& version : _versions)
        _r.push_back(version.serialize());
    return _r;
}

void Versions::save()
{
    _initDir();	
    for (const auto& version : _versions)
        files::write(_directory + files::sep() + version.paddedVersion(), version.serialize().dump());
}

void Versions::createNew(const json& versionData, const std::string& notes)
{
    if (_versions.empty())
        this->read(1);
    int version = 0;
    if (!_versions.empty())
        version = _versions[0].version() + 1;
    version ++;
    Version v(version, notes);
    v.setSavedData(versionData);
    _versions.push_back(v);
    this->save();

    if (FxPipe::get()->projectSettings().versioningSystem.type == VersioningSystem::GLOBAL)
    {
        ml::Vec cmds = str::split(FxPipe::get()->projectSettings().versioningSystem.commands, "\n");
        for (auto pc : cmds)
        {
            if (pc.empty())
                continue;
            try
            {
                pc = str::replace(pc, "$notes", notes);
                boost::process::child c(pc, boost::process::std_out > boost::process::null, boost::process::std_err > boost::process::null);
                c.wait();
            }
            catch(const std::exception& e)
            {
                lg("Error in Versions::createNew() : \nThis process failed : " + pc);
                lg(e.what());
            }
        }
    }
}
