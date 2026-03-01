#include "./FxpipeKeybinds.h"
#include "./FxPipe.h"
#include "./Task.h"
#include "./TaskView.h"

void FxpipeKeybinds::add(const std::string& key, std::function<void()> func)
{
    auto m = fxpipe::get()->main();

    auto f = [this, func]()-> bool
    {
        func();
        return true;
    };
    m->addKeybind(key, f);
}

void FxpipeKeybinds::add(const std::string& key, std::function<void(Task*)> func)
{
    auto m = fxpipe::get()->main();
    auto f = [this, func]()-> bool
    {
        auto task = fxpipe::get()->activeTask();
        if (task)
        {
            func(task);
            return true;
        }
        return false;
    };

    m->addKeybind(key, f);
}

void FxpipeKeybinds::add(const std::string& key, std::function<void(TaskView*)> func)
{
    auto m = fxpipe::get()->main();
    auto f = [this, func]()-> bool
    {
        auto view = fxpipe::get()->activeTask()->parentView();
        if (view)
        {
            func(view);
            return true;
        }
        return false;
    };

    m->addKeybind(key, f);
}
