#pragma once
#include "./Tasks.h"
#include "./Task.h"

template <typename T>
ml::Ret<T*> Tasks::task(const std::string&id)
{
    for (const auto& task : _tasks)
    {
        if (task->id() == id)
            return ml::ret::success((T*)task.get());
    }
    return ml::ret::fail<T*>("Task " + id + " not found.");
}
