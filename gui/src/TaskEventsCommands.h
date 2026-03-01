#pragma once
#include "mlgui.2/src/Property.h"
#include "mlgui.2/src/PropertiesManager.h"
#include "commands/CommandsManager.h"
#include "mlgui.2/src/GuiCommand.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace ml
{
    class Box;
    class Label;
}

struct EventProp
{
    ml::Property* prop = nullptr;
    ml::Box* box = nullptr;
};

class TaskEventsCommands
{
    public : 
        TaskEventsCommands(ml::Box* parent);
        ~TaskEventsCommands() = default;

        void draw(ml::Box* parent);
        void createMenu();
        void createCommands();

        void onCreate();
        void onRemove();

        ml::StringProperty* createEvent(const std::string& event, const std::string& cmds = "");

        json serialize() const;
        void deserialize(const json& data);

        void clear();

        //return nullptr if not founded
        ml::StringProperty* eventProp(const std::string& event);

    private: 
        ml::PropertiesManager _props;
        ml::Box* _cmdsBox;
        ml::Label* _nothingYetLabel = nullptr;

        ml::StringProperty _newEvent;
        void _setActiveEvent(ml::Property* prop, ml::Box* box) ;
        void _createNothingLabel();

        EventProp _activeEvent = {nullptr, nullptr};
};
