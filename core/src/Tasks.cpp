#include "./Tasks.h"
#include "./Task.h"
#include "./FxPipe.h"

json Tasks::serialize() const
{
    json _r = json::array();	
    for (auto& task : _tasks)
        _r.push_back(task->serialize());
    return _r;
}

void Tasks::deserialize(const json& data)
{
    this->clear();
    for (auto& task : data)	
    {
        if (!task.contains("type") || task["type"] == "Task")
        {
            this->createTask<Task>(task);
        }
        //else if ... (you can put the different subtypes heeere...)
    } 
}

bool Tasks::remove(const std::string&id)
{
    for (auto &task : _tasks)
    {
        if (task->id() == id)
        {
            _tasks.remove(task);
            return true;
        }
    }
    return false;
}
bool Tasks::remove(Task* task){return this->remove(task->id());}

void Tasks::move(const std::string& id,int movement)
{
    if (movement == 0) return;
    
    auto it = _tasks.begin();
    int currentIndex = 0;
    
    // Find the task with the given id
    while (it != _tasks.end())
    {
        if ((*it)->id() == id)
            break;
        ++it;
        ++currentIndex;
    }
    
    if (it == _tasks.end())
        return; // Task not found
    
    int newIndex = currentIndex + movement;
    
    // Clamp the new index to valid range
    if (newIndex < 0)
        newIndex = 0;
    if (newIndex >= static_cast<int>(_tasks.size()))
        newIndex = _tasks.size() - 1;
    
    if (newIndex == currentIndex)
        return;
    
    // Extract the task
    auto task = std::move(*it);
    _tasks.vec.erase(it);
    
    // Insert at new position
    auto insertIt = _tasks.begin();
    std::advance(insertIt, newIndex);
    _tasks.insert(insertIt, std::move(task));
}

void Tasks::move(Task* task, int movement){this->move(task->id(), movement);}

void Tasks::swap(const std::string& from,const std::string& to)
{
    if (from == to) return;
    
    auto fromIt = _tasks.begin();
    auto toIt = _tasks.begin();
    int fromIndex = -1;
    int toIndex = -1;
    int currentIndex = 0;
    
    // Find both tasks
    for (auto it = _tasks.begin(); it != _tasks.end(); ++it, ++currentIndex)
    {
        if ((*it)->id() == from)
        {
            fromIt = it;
            fromIndex = currentIndex;
        }
        if ((*it)->id() == to)
        {
            toIt = it;
            toIndex = currentIndex;
        }
    }
    
    // If either task not found, return
    if (fromIndex == -1 || toIndex == -1)
        return;
    
    // Swap the tasks
    std::iter_swap(fromIt, toIt);
}

ml::Vec<std::string> Tasks::ids() const
{
    ml::Vec<std::string> _ids;
    for (const auto& task : _tasks)
        _ids.push_back(task->id());
    return _ids;
}

json Tasks::ids_json() const
{
    json _r = json::array();
    for (const auto& task : _tasks)
        _r.push_back(task->id());
    return _r;
}

void Tasks::cleanup()
{
    lg("Cleaning up " << _tasks.size() << " tasks from the task " << this->parent()->id());
    auto last = _tasks.size() - 1;
    for (int i = last; i >= 0; i--)
    {
        lg("Checking if task idx " << i << " is null : " << _tasks[i].get());
        if (!_tasks[i])
        {
            lg("It is ! Removing it...");
            _tasks.removeByIndex(i);
        }
#ifdef mydebug
        else
        {
            lg("Non null task idx " << i << " is not null and got the id : " << _tasks[i]->id());
        }
#endif
    }

    lg("Cleanedup.");
    lg("Now there are " << _tasks.size() << " tasks");
}

std::unique_ptr<Task>& Tasks::add(std::unique_ptr<Task>&& task)
{
    lg("Tasks::add(std::unique_ptr<Task>&& task=" << task.get() << ")");
    auto* oldparent = task->parent();
    if (oldparent == this)
    {
        lg("The parent is the same, notning to do.");
        return FxPipe::get()->nullTask();
    }

    lg("Pushing the task to _tasks with std::move");
#ifdef mydebug 
    std::string id = task->id();
#endif
    task->setParent(this);
    _tasks.push_back(std::move(task));
#ifdef mydebug
    lg("Task " << id << " pushed.");
#endif
    oldparent->cleanup();

    lg("Returning the last task (the pushed one) : " << _tasks.back().get() << " with id " << _tasks.back()->id());
    return _tasks.back();
}

std::unique_ptr<Task>& Tasks::add(std::unique_ptr<Task>& task)
{
    return this->add(std::move(task));
}

ml::Ret<Task*> Tasks::add(const std::string& id)
{
    lg("Tasks::add(" << id << ")");
    auto& task = FxPipe::get()->taskUniqPtr(id);
    lg("Task returned from the id : " << task.get());
    if (FxPipe::get()->isTaskNull(task))
        return ml::ret::failure<Task*>("Task " + id + " not found.");

    auto& reparented_task = this->add(task);
    if (FxPipe::get()->isTaskNull(reparented_task))
        return ml::ret::failure<Task*>("Task " + id + " not found after being parented. This should not happen.");

    return ml::ret::success(reparented_task.get());
}

void Tasks::clear()
{
    _tasks.clear();	
}

ml::Ret<> Tasks::reorder(const ml::Vec<std::string>& ids)
{
    if (ids.size() != _tasks.size())
        return ml::ret::fail("Size mismatch with the ids given : " + std::to_string(ids.size()) + " vs the tasks size : " + std::to_string(_tasks.size())); // Size mismatch, cannot reorder
    
    // Create a map of id to task for quick lookup
    std::unordered_map<std::string, std::unique_ptr<Task>> taskMap;
    for (auto& task : _tasks)
        taskMap[task->id()] = std::move(task);
    
    // Clear the current tasks
    _tasks.clear();
    
    // Reorder according to the provided ids
    for (const auto& id : ids)
    {
        auto it = taskMap.find(id);
        if (it != taskMap.end())
            _tasks.push_back(std::move(it->second));
    }

    return ml::ret::success();
}

ml::Vec<Task*> Tasks::allTasks()
{
    ml::Vec<Task*> tasks;	
    for (auto& task : _tasks)
    {
        tasks.push_back(task.get());
        tasks.concat(task->subtasks().allTasks());
    }
    return tasks;
}
