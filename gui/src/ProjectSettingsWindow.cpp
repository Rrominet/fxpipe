#include "./ProjectSettingsWindow.h"
#include "./FxPipe.h"
#include "./MainWindow.h"
#include "mlgui.2/src/Label.h"
#include <unordered_map>

#include "mlgui.2/src/Scrollable.hpp"

#define CONSTRUCT_PROPS \
    _name("Name", "", "Name of your Project"), \
    _description("Description", "", "A detailed description of the project if needed."), \
    _versioningType("Versioning Tool", "Global", "The versioning tool used in this project", ml::Vec<std::string>{"Global", "Per Asset (MASTER)"}),\
    _globalCommands("Command", "", "The commands to executes when a new version is created. You can use the variable $notes to get the version notes piped into it.")


std::unordered_map<std::string, std::string> versioningTypeHelps = {
    {"Global", "Use a global system version for the project that only save the fxpipe file as it is.\n You need to bind a command to it to actually have a version system.\n(Typically : git)"},
    {"Per Asset (MASTER)", "Keep trak of the versions per Asset (or here task) with a system called the \"MASTER System\".\nThis is particulary adapted for binaries based projects like photos, videos, 3D, etc."},
};

ProjectSettingsWindow::ProjectSettingsWindow(ml::App* app) : ml::Window(app), CONSTRUCT_PROPS
{
	
}

ProjectSettingsWindow::ProjectSettingsWindow(ml::App* app,ml::Window* parent) : ml::Window(app, parent), CONSTRUCT_PROPS
{
	
}

ProjectSettingsWindow::~ProjectSettingsWindow()
{
	
}

void ProjectSettingsWindow::init()
{
    ml::Window::init();
    this->setSize(550, 740);
    this->setTitle("Project Settings");

    _main->createLabel("Your project settings :");
    _main->appendProp(&_name);
    _main->appendProp(&_description, ml::MULTILINE_ENTRY);
    _main->createSeparator();
    _main->appendEnumProp(&_versioningType);
    _main->appendProp(&_globalCommands, ml::MULTILINE_ENTRY);
    _versioningTypeHelp = _main->createLabel(versioningTypeHelps["Global"]).get();

    auto cb = [this](const json& res)
    {
        if (!res.contains("data"))
            return;
        fxpipe::get()->setProjectSettings(res["data"]);
        fxpipe::get()->fxpipeW()->updateProjectSettings();
        this->hide();
    };

    auto valid = fxpipe::get()->createBackendCommand(fxpipe::get()->backend(), "Set Project", "set-project", "set-project",
            [this]{return this->data();}, cb);

    _main->appendCommand((ml::Command*)valid.get());
    _setEvents();
}

void ProjectSettingsWindow::_setEvents()
{
    _versioningType.addOnUpdate([this]{
                _versioningTypeHelp->setText(versioningTypeHelps[_versioningType.value()]);
            });

    this->addKeybind("ctrl enter", [this]{
                fxpipe::get()->cmds().exec("set-project");
                this->hide();
                return true;
            });
}

json ProjectSettingsWindow::data()
{
    json _r;	
    _r["name"] = _name.value();
    _r["description"] = _description.value();
    _r["versioningSystem"] = json::object();

    if (_versioningType.value() == "Global")
        _r["versioningSystem"]["type"] = 0;
    else if (_versioningType.value() == "Per Asset (MASTER)")
        _r["versioningSystem"]["type"] = 1;
    else 
        _r["versioningSystem"]["type"] = -1;

    _r["versioningSystem"]["commands"] = _globalCommands.value();
    return _r;
}

void ProjectSettingsWindow::setFromData(const json& data)
{
    if (data.contains("name"))
        _name.set(data["name"].get<std::string>());
    if (data.contains("description"))
        _description.set(data["description"].get<std::string>());
    if (data.contains("versioningSystem"))
    {
        if (data["versioningSystem"]["type"].get<int>() == 0)
            _versioningType.set(_S"Global");
        else if (data["versioningSystem"]["type"].get<int>() == 1)
            _versioningType.set(_S"Per Asset (MASTER)");

        if (data["versioningSystem"].contains("commands"))
            _globalCommands.set(data["versioningSystem"]["commands"].get<std::string>());
    }
}

void ProjectSettingsWindow::reset()
{
    _name.set(_S"");	
    _description.set(_S"");
    _versioningType.set(_S"Global");
    _globalCommands.set(_S"");
}
