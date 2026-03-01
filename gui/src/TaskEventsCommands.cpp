#include "./TaskEventsCommands.h"
#include "mlgui.2/src/Box.h"
#include "mlgui.2/src/Box.hpp"
#include "mlgui.2/src/AskPropertyDialog.h"
#include "mlgui.2/src/Property.h"
#include "mlgui.2/src/enums.h"

TaskEventsCommands::TaskEventsCommands(ml::Box* parent): 
    _newEvent("New Event ID", "", "Could be open, close, name, description, etc.")
{
    this->createCommands();
    this->draw(parent);	
}

void TaskEventsCommands::_createNothingLabel()
{
    _nothingYetLabel = _cmdsBox->createLabel("Add an Command with Right Click -> New").get();
    _nothingYetLabel->addCssClass("nothing-yet");
    _nothingYetLabel->setHAlign(ml::CENTER);
}

void TaskEventsCommands::draw(ml::Box* parent)
{
    _cmdsBox = parent->createBox().get();
    _cmdsBox->setVExpand();
    _createNothingLabel();
    this->createMenu();
}

void TaskEventsCommands::createMenu()
{
    auto menu = ml::app()->menusFactory().create("task-events-commands", "Events Commands");
    menu->addCommand("task-create-event-cmd");
    menu->addCommand("task-remove-event-cmd");

    _cmdsBox->setContextMenu("task-events-commands");
}

void TaskEventsCommands::createCommands()
{
    auto cmd = ml::app()->cmds().createCommand<ml::GuiCommand>("New", "task-create-event-cmd").get();	
    cmd->setExec([this](const std::any&){this->onCreate();});

    cmd = ml::app()->cmds().createCommand<ml::GuiCommand>("Remove", "task-remove-event-cmd").get();
    cmd->setExec([this](const std::any&){this->onRemove();});
}

void TaskEventsCommands::onCreate()
{
    auto onvalid = [this]
    {
        this->createEvent(_newEvent.value());    
    };

    ml::app()->ask(&_newEvent, onvalid);
}

ml::StringProperty* TaskEventsCommands::eventProp(const std::string& event)
{
    for (auto& prop : _props.props())
    {
        std::string sevent;
        try
        {
            sevent = std::any_cast<std::string>(prop->attribute("event"));
        }
        catch(const std::exception& e)
        {
            continue;
        }
        if (event == sevent)
            return (ml::StringProperty*)prop.get();
    }
    return nullptr;
}

ml::StringProperty* TaskEventsCommands::createEvent(const std::string& event, const std::string& cmds)
{
    _nothingYetLabel->hide();
    auto prop = this->eventProp(event);
    if (!prop)
    {
        prop = _props.create<ml::StringProperty>("Commands for " + event);
        prop->setAttribute("event", event);
    }
    if (!cmds.empty())
        prop->set(cmds);

    auto widget = _cmdsBox->appendProp(prop, ml::MULTILINE_ENTRY);
    widget->box()->addCssClass("event-cmds");

    widget->box()->addEventListener(ml::MOUSE_ENTER, [this, prop, widget](ml::EventInfos&){
            _setActiveEvent(prop, widget->box().get());
            });
    return prop;
}

void TaskEventsCommands::onRemove()
{
    if (!_activeEvent.prop || !_activeEvent.box)
        return;	

    _props.destroy(_activeEvent.prop);
    _activeEvent.box->remove();
}

void TaskEventsCommands::_setActiveEvent(ml::Property* prop, ml::Box* box)
{
    if (_activeEvent.box)
    {
        if (_activeEvent.box == box)
            return;
        _activeEvent.box->removeCssClass("active");
    }

    if(_activeEvent.prop && _activeEvent.prop == prop)
        return;

    _activeEvent.prop = prop;
    _activeEvent.box = box;
    if (_activeEvent.box)
        _activeEvent.box->addCssClass("active");	
}

json TaskEventsCommands::serialize() const
{
    json r = json::object();
    for (auto& prop : _props.props())
    {
        auto key = std::any_cast<std::string>(prop->attribute("event"));
        r[key] = json::array();
        ml::Vec<std::string> cmds = str::split(prop->asString(), "\n");
        for (const auto& cmd : cmds)
        {
            if (cmd.empty())
                continue;
            r[key].push_back(cmd);
        }
    }
    return r;
}

void TaskEventsCommands::deserialize(const json& data)
{
    this->clear();
    for (const auto& prop : data.items())
    {
        json value = prop.value();
        std::string propval = "";
        for (const auto& cmd : value)
            propval += cmd.get<std::string>() + "\n";
        this->createEvent(prop.key(), propval);
    }
}

void TaskEventsCommands::clear()
{
    _cmdsBox->clear();
    _props.clear();	
    _createNothingLabel();
}
