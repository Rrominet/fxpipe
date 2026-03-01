#pragma once
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Ret.h"
#include "vec.h"

class Task;
class Tasks
{
    public : 
        Tasks(Task* parent=nullptr): _parent(parent) {}
        template <typename T=Task>
            T* createTask(const json& data=json::object())
            {
                auto task = std::make_unique<T>(data);
                task->setParent(this);
                auto id = task->id();
                _tasks.push(std::move(task));
                return dynamic_cast<T*>(_tasks.back().get());
            }

        template <typename T=Task>
            ml::Ret<T*> task(const std::string&id);

    json serialize() const;
    void deserialize(const json& data);

    bool remove(const std::string&id);
    bool remove(Task* task);

    //movement is negative for up, positive for down 
    void move(const std::string& id, int movement);
    void move(Task* task, int movement);
    void swap(const std::string& from, const std::string& to);

    ml::Vec<std::string> ids() const;
    json ids_json() const;

    ml::Vec<std::unique_ptr<Task>>& tasks(){return _tasks;}
    const ml::Vec<std::unique_ptr<Task>>& tasks()const {return _tasks;}

    bool empty()const {return _tasks.empty();}
    size_t size()const {return _tasks.size();}

    //it will remove the task from its old parent and it will invalidate the task unique_ptr passed as argument! Use the returned one after calling this.
    std::unique_ptr<Task>& add(std::unique_ptr<Task>&& task);
    std::unique_ptr<Task>& add(std::unique_ptr<Task>& task);

    //return the added task
    ml::Ret<Task*> add(const std::string& id);

    void clear();

    ml::Ret<> reorder(const ml::Vec<std::string>& ids);

    //remove any nullptr in tasks - that can happen when reparenting
    void cleanup();

    private : 
        ml::Vec<std::unique_ptr<Task>> _tasks;
        Task* _parent = nullptr; //bp cg

    public : 
#include "./Tasks_gen.h"
};
