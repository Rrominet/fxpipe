#pragma once
#include "mlgui.2/src/Window.h"
#include "mlgui.2/src/Property.h"
#include "./Task.h"

namespace ml
{
    class Collapsable;
}

class TaskEventsCommands;
class TaskSettingsWindow : public ml::Window
{
    public : 
        enum Mode
        {
            CREATE,
            MODIFY,
        };
        TaskSettingsWindow(ml::App* app);
        TaskSettingsWindow(ml::App* app, ml::Window* parent);
        virtual ~TaskSettingsWindow();

        virtual void init() override;
        void setFromTask(Task* task);
        void reset();
        
        Task::Status status();
        Task::Priority priority();

        json data();

        void setParent(const std::string& id, const std::string& name); 
        void setMode(Mode mode);

    protected : 
        ml::EnumProperty _type;

        std::string _parentId;
        ml::Label* _parentName;

        ml::StringProperty _name;
        ml::StringProperty _description;
        ml::EnumProperty _category;
        ml::EnumProperty _status;
        ml::EnumProperty _priority;
        ml::BoolProperty _archived;
        ml::StringProperty _deadline;

        ml::Button* _cmdButton = nullptr;

        ml::Collapsable* _eventsCmdsCollapsable = nullptr;
        std::unique_ptr<TaskEventsCommands> _taskEventsCommands;

        Mode _mode = CREATE; //bp cg

    public : 
        #include "TaskSettingsWindow_gen.h"
};


