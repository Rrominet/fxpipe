#include "./Stats.h"
#include "./FxPipe.h"
#include "files.2/files.h"
#include "mlTime.h"


void Stats::_saveDate(const std::string& dateKey)
{
    if (_directory.empty())
    {
        lg("Stats::_saveDate(" << dateKey << ") error : _directory is empty");
        return;
    }
    auto filepath = _directory + files::sep() + dateKey;	
    try
    {
        files::write(filepath, _data[dateKey].dump());
    }
    catch(const std::exception& e)
    {
        lg("Stats::_saveDate(" << dateKey << ") error : ");
        lg(e.what());
    }
}

ml::Ret<> Stats::save()
{
    auto _r = _initDir();	
    if(!_r.success)
        return _r;
    for (auto d : _data.items())
        _saveDate(d.key());
    return _r;
}

ml::Ret<> Stats::read(int quantity)
{
    auto _r = _initDir();	
    if(!_r.success)
        return _r;
    try
    {
        lg("Stats::read(" << quantity << ") : Listing files in " << _directory);
        auto ls = files::ls(_directory, files::NAME_INV);
        if (ls.size() < quantity)
            quantity = ls.size();
        for (int i = 0; i < quantity; i++)
        {
            try
            {
                _data[files::name(ls[i])] = json::parse(files::read(ls[i]));
            }
            catch(const std::exception& e)
            {
                lg("Error while reading the file " << ls[i]);
                lg(e.what());
            }
        }
    }
    catch(const std::exception& e)
    {
        lg(e.what());
        return ml::ret::fail("Stats::read error () : " + _S(e.what()));
    }
    return _r;
}

ml::Ret<> Stats::saveTaskStatusChange(Task* task)
{
    json d = json::object();
    d["id"] = task->id();
    d["name"] = task->name();
    d["status"] = task->status();
    d["time"] = ml::time::now();

    auto dateKey = ml::time::asStringReverse(ml::time::now());
    if (!_data.contains(dateKey))
        _data[dateKey] = json::array();
    _data[dateKey].push_back(d);
    return this->save();
}

ml::Ret<> Stats::_initDir()
{
    if (FxPipe::get()->currentProject().empty())
        return ml::ret::fail("Stats::_initDir() : The project needs to be saved first.");
    auto d = files::parent(FxPipe::get()->currentProject());
    if (files::isDir(d + files::sep() + "." + files::name(FxPipe::get()->currentProject()) + "__stats"))
    {
        _directory = d + files::sep() + "." + files::name(FxPipe::get()->currentProject()) + "__stats";
        return ml::ret::success();
    }

    if (!files::exists(d))
        return ml::ret::fail("Stats::_initDir() : " + d + " does not exist.");
    if (!files::isDir(d))
        return ml::ret::fail("Stats::_initDir() : " + d + " is not a directory.");

    if (!files::mkdir(d + files::sep() + "." + files::name(FxPipe::get()->currentProject()) + "__stats"))
        return ml::ret::fail("Stats::_initDir() : Could not create " + d + files::sep() + "." + files::name(FxPipe::get()->currentProject()) + "__stats");
    _directory = d + files::sep() + "." + files::name(FxPipe::get()->currentProject()) + "__stats";
    return ml::ret::success();
}

