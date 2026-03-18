#pragma once
#include "./Tasks.h"
#include "./Task.h"
#include "./ProjectSettings.h"
#include "./Versions.h"
#include "./Stats.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Ret.h"

class FxPipe
{
    public : 
        FxPipe(int argc,char* argv[]);
        virtual ~FxPipe() = default;

        static FxPipe* get();

        json serialize() const;
        void deserialize(const json& data);

        ml::Ret<> open(const std::string& path);
        ml::Ret<> save(const std::string& path);

        //find a task reccursively
        std::unique_ptr<Task>& taskUniqPtr(const std::string&id, Tasks* parent=nullptr);
        std::unique_ptr<Task>& taskUniqPtr(const std::string&id, Task* parent);
        std::unique_ptr<Task>& taskUniqPtr(const std::string&id, const std::string& parentId);
        
        //find a task reccursively
        template <typename T=Task>
            ml::Ret<T*> task(const std::string&id, Tasks* parent=nullptr)
            {
                auto& task = this->taskUniqPtr(id, parent);
                if (this->isTaskNull(task))
                    return ml::ret::failure<T*>("Task " + id + " not found.");
                auto t = dynamic_cast<T*>(task.get());
                if(!t)
                    return ml::ret::failure<T*>("Task " + id + " is not of type (or subtype) " + typeid(T).name());
                return ml::ret::success(t);
            }

        template <typename T=Task>
            ml::Ret<T*> task(const std::string&id, Task* parent)
            {
                return this->task<T>(id, &parent->subtasks());
            }
        //
        //find a task reccursively
        template <typename T=Task>
            ml::Ret<T*> task(const std::string&id, const std::string& parentId)
            {
                auto tasks = this->task<Task>(parentId);
                if (!tasks.success)
                    ml::ret::failure("Parent task " + parentId + " not found.");

                return this->task<T>(id, tasks.value);
            }

        ml::Vec<Task*> tasks(const ml::Vec<std::string>& ids);

        void treatArgs(int argc, char* argv[]);
        bool isTaskNull(std::unique_ptr<Task>& task) const;
        ml::Vec<std::string> asList(const json& data, const std::string& key="id");

        std::unique_ptr<Task>& nullTask() {return _nullTask;};
        ml::Vec<Task*> allTasks();

        //public client interface
    public : 
        json _createTask(const json& args);
        json _task(const json& args);
        json _removeTask(const json& args);
        json _open(const json& args);
        json _save(const json& args);
        json _move(const json& args);
        json _swap(const json& args);
        json _reparent(const json& args);
        json _emitTaskEvent(const json& args);
        json _getAllData(const json& args);
        json _newProject(const json& args);
        json _reorderTasks(const json& args);
        json _setProjectSettings(const json& args);
        json _getProjectSettings(const json& args);
        json _createNewVersion(const json& args);
        json _listVersions(const json& args);
        json _getStats(const json& args);
        json _currentStatsDone(const json& args);
        json _search(const json& args);

    private : 
        void reg();
        Tasks _tasks; //bp cg
        ml::Vec<std::string> _categories;
        ProjectSettings _projectSettings; //bp cg

        std::unique_ptr<Task> _nullTask;

        //could be used or not depending of the versionioning system used.
        //Typically used with the global ones like git.
        Versions _versions; //bp cgs

        std::string _currentProject; //bp cg
        Stats _stats; //bp cg

    public : 
#include "./FxPipe_gen.h"
};
