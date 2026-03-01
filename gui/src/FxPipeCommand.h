#pragma once
#include "mlgui.2/src/GuiCommand.h"

class Task;
class TaskView;
class FxPipeCommand : public ml::GuiCommand
{
    public : 
        FxPipeCommand(): ml::GuiCommand() {}
        virtual ~FxPipeCommand() = default;

        void setExecForActiveTask(const std::function<void(Task* task)>& f);
        void setExecForSelectedTasks(const std::function<void(Task* task)>& f);
        void setExecForSelectedTasks(const std::function<void(const ml::Vec<Task*>& tasks)>& f);
        void setExecForActiveTaskView(const std::function<void(TaskView* task)>& f);
        void setExecGlobal(const std::function<void()>& f);
};
