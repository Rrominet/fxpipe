#include "./FxPipeCommand.h"
#include "./Task.h"
#include "./TaskView.h"
#include "./FxPipe.h"

void FxPipeCommand::setExecForActiveTask(const std::function<void(Task* task)>& f)
{
    auto _f = [this, f](const std::any& args)
    {
        auto task = fxpipe::get()->activeTask();
        if (task)
            f(task);
    };
    this->setExec(_f);
}

void FxPipeCommand::setExecForSelectedTasks(const std::function<void(Task* task)>& f)
{
    auto _f = [this, f](const std::any& args)
    {
        for (auto task : fxpipe::get()->selectedTasks())
            f(task);
    };
    this->setExec(_f);
}

void FxPipeCommand::setExecForSelectedTasks(const std::function<void(const ml::Vec<Task*>& tasks)>& f)
{
    auto _f = [this, f](const std::any& args)
    {
        f(fxpipe::get()->selectedTasks());
    };
    this->setExec(_f);
}

void FxPipeCommand::setExecForActiveTaskView(const std::function<void(TaskView* task)>& f)
{
    auto _f = [this, f](const std::any& args)
    {
        auto view = fxpipe::get()->activeTaskView();;
        if (view)
            f(view);
    };
    this->setExec(_f);
}

void FxPipeCommand::setExecGlobal(const std::function<void()>& f)
{
    auto _f = [this, f](const std::any& args)
    {
        f();
    };
    this->setExec(_f);
}
