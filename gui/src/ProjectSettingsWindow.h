#pragma once
#include "mlgui.2/src/Window.h"
#include "mlgui.2/src/Property.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class ProjectSettingsWindow : public ml::Window
{
    public : 
        ProjectSettingsWindow(ml::App* app);
        ProjectSettingsWindow(ml::App* app, ml::Window* parent);
        virtual ~ProjectSettingsWindow();

        virtual void init() override;

        json data();
        void setFromData(const json& data);

        void reset();

    protected : 
        ml::StringProperty _name; 
        ml::StringProperty _description;
        ml::EnumProperty _versioningType;
        ml::Label* _versioningTypeHelp = nullptr;
        ml::StringProperty _globalCommands;

        ml::Button* _cmdButton = nullptr;

        void _setEvents();
};
