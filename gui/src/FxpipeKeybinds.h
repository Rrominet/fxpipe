#pragma once
#include <string>
#include <functional>

class Task;
class TaskView;
class FxpipeKeybinds
{
    public:
        FxpipeKeybinds() = default;
        ~FxpipeKeybinds() = default;

        void add(const std::string& key, std::function<void()> func);
        void add(const std::string& key, std::function<void(Task*)> func);
        void add(const std::string& key, std::function<void(TaskView*)> func);
};
