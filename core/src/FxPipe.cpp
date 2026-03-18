#include "./FxPipe.h"
#include <memory>
#include "ipc.h"
#include "files.2/files.h"
#include "mlprocess.h"

#include "./Tasks.hpp"
#include "str.h"

FxPipe* _instance = nullptr;

FxPipe::FxPipe(int argc,char* argv[])
{
    _instance = this;
    this->reg();
    this->treatArgs(argc, argv);
    ipc::receive();
}

void FxPipe::treatArgs(int argc,char* argv[])
{
    if(!argc)
        return;
    if (!argv)
        return;

    auto args = args::nparse(argc, argv);
#ifdef mydebug
    for (const auto& arg : args)
    {
        lg(arg.first << " : " << arg.second);
    }
#endif
    if (args.find("0") != args.end())
        this->open(args["0"]);
}

FxPipe* FxPipe::get()
{
    return _instance;
}

void FxPipe::reg()
{
    ipc::reg("test", [this](const json& args){json _r; _r["message"] = "test worked."; ipc::success(_r); return _r;}); 

    ipc::reg("create-or-modify-task", [this](const json& args){return _createTask(args);}, {}, {"id", "type", "category", "name", "description", "deadline", "color", "status", "priority", "price", "cmd", "files", "version", "includeFilesInCmd", "onchangedCmds", "parent", "archived"});
    ipc::reg("task", [this](const json& args){return _task(args);}, {"id"}, {"parent"});
    ipc::reg("remove-task", [this](const json& args){return _removeTask(args);}, {"id"});

    ipc::reg("open", [this](const json& args){return _open(args);}, {"filepath"});
    ipc::reg("save", [this](const json& args){return _save(args);}, {"filepath"});

    ipc::reg("move", [this](const json& args){return _move(args);}, {"id", "movement"});
    ipc::reg("swap", [this](const json& args){return _swap(args);}, {"from", "to"});
    ipc::reg("reorder-tasks", [this](const json& args){return _reorderTasks(args);}, {"tasks"}, {"parent"});

    ipc::reg("reparent", [this](const json& args){return _reparent(args);}, {"id"}, {"parent"});
    ipc::reg("emit-task-event", [this](const json& args){return _emitTaskEvent(args);}, {"id","event"});
    ipc::reg("get-all-data", [this](const json& args){return _getAllData(args);});
    ipc::reg("new-project", [this](const json& args){return _newProject(args);});
    ipc::reg("set-project", [this](const json& args){return _setProjectSettings(args);}, {}, {"name", "description", "versioningType"});
    ipc::reg("project-settings", [this](const json& args){return _getProjectSettings(args);});

    ipc::reg("create-new-version", [this](const json& args){return _createNewVersion(args);}, {}, {"notes"});
    ipc::reg("list-versions", [this](const json& args){return _listVersions(args);}, {}, {"quantity"});

    ipc::reg("stats", [this](const json& args){return _getStats(args);}, {}, {"quantity"});
    ipc::reg("current-stats-done", [this](const json& args){return _currentStatsDone(args);});
    ipc::reg("search", [this](const json& args){return _search(args);}, {"query"});

#ifdef mydebug
    ipc::logAll();
#endif
}

//it's create or modify actually
json FxPipe::_createTask(const json& args)
{
    json _r;
    Task* task = nullptr;
    Tasks* tasks = &_tasks;
    if (args.contains("parent") && args["parent"].get<std::string>() != "")
    {
        auto ret = this->task(args["parent"].get<std::string>());
        if (!ret.success)
        {
            ipc::error(_r, ret.message);
            return _r;
        }

        tasks = &ret.value->subtasks();
    }

    if (args.contains("id"))
    {
        auto ret = tasks->task(args["id"].get<std::string>());
        if (ret.success)
        {
            task = ret.value;
            task->deserialize(args, true);
        }
    }

    if (!task)
    {
        if (args.contains("type"))
        {
            if (args["type"] == "Task")
                task = tasks->createTask<Task>(args);
        }
        else 
        {
            lg("Subtasks ptr " << tasks);
            task = tasks->createTask<Task>(args);
        }
    }

    _r["data"] = task->serialize();
    return _r;
}

json FxPipe::_task(const json& args)
{
    json _r	;
    ml::Ret<Task*> tret;
    if (args.contains("parent"))
        tret = this->task(args["id"].get<std::string>(), args["parent"].get<std::string>());
    else 
        tret = this->task(args["id"].get<std::string>());

    if (!tret.success)
        ipc::error(_r, tret.message); 
    else 
    {
        _r["data"] = tret.value->serialize();
    }
    return _r;
}

json FxPipe::_removeTask(const json& args)
{
    json _r;	
    auto tasks = this->tasks(this->asList(args, "id"));
    _r["data"] = json::array();
    for (auto t : tasks)
    {
        auto removedId = t->id();
        bool removed = t->parent()->remove(t);
        if (!removed)
            ipc::error(_r, "The task " + args["id"].get<std::string>() + " founded but not removed."); 
        else 
            _r["data"].push_back(removedId);
    }
    ipc::success(_r);
    return _r;
}

json FxPipe::_open(const json& args)
{
    json _r;
    auto res = this->open(args["filepath"].get<std::string>());
    if (!res.success)
        ipc::error(_r, res.message);
    _r["data"] = this->serialize();
    ipc::success(_r);
    return _r;
}

json FxPipe::_save(const json& args)
{
    json _r;
    auto res = this->save(args["filepath"].get<std::string>());
    if (!res.success)
        ipc::error(_r, res.message);
    ipc::success(_r);
    return _r;
}

json FxPipe::_move(const json& args)
{
    json _r;	
    auto t = this->task(args["id"].get<std::string>());
    if (!t.success)
    {
        ipc::error(_r, t.message);
        return _r;
    }

    t.value->parent()->move(t.value, args["movement"].get<int>());
    _r["data"] = t.value->parent()->ids_json();
    return _r;
}

json FxPipe::_swap(const json& args)
{
    json _r;	
    Tasks* tasks = &_tasks;
    if (args.contains("parent"))
    {
        auto parent = this->task(args["parent"].get<std::string>());
        if (!parent.success)
        {
            ipc::error(_r, parent.message);
            return _r;
        }
        tasks = &parent.value->subtasks();
    }

    tasks->swap(args["from"].get<std::string>(), args["to"].get<std::string>());
    _r["data"] = tasks->ids_json();
    return _r;
}

json FxPipe::serialize() const
{
    json _r;	
    _r["tasks"] = _tasks.serialize();
    _r["categories"] = json::array();
    for (const auto& cat : _categories)
        _r["categories"].push_back(cat);
    _r["project-settings"] = _projectSettings.serialize();
    return _r;
}

void FxPipe::deserialize(const json& data)
{
    if (data.contains("tasks"))
        _tasks.deserialize(data["tasks"]);    
    if (data.contains("categories"))
    {
        _categories.clear();
        for (auto& cat : data["categories"])
            _categories.push_back(cat);
    }
    if (data.contains("project-settings"))
        _projectSettings.deserialize(data["project-settings"]);
}

ml::Ret<> FxPipe::open(const std::string& path)
{
    lg("opening " << path);
    std::string data;
    try
    {
        data = files::read(path);
    }
    catch(const std::exception& e)
    {
        lg(e.what());
        return ml::ret::fail("[FxPipe::open] Could not open the file " + path + " : " + e.what());
    }

    json jdata;
    try
    {
        jdata = json::parse(data);
    }
    catch(const std::exception& e)
    {
        lg(e.what());
        return ml::ret::fail("[FxPipe::open] couldn't parse the json data in the file " + path + " : " + e.what());
    }
    try
    {
        this->deserialize(jdata);
    }
    catch(const std::exception& e)
    {
        lg(e.what());
        return ml::ret::fail("[FxPipe::open] couldn't deserialize the data : " + jdata.dump(4) + "\nMore infos on the error : " + e.what());
    }
     _currentProject = path;
     auto statsres = _stats.read();
     if (!statsres.success)
         return statsres;
    return ml::ret::success();
}

ml::Ret<> FxPipe::save(const std::string& path)
{
    json jdata = this->serialize();	
    try
    {
        files::write(path, jdata.dump());
    }
    catch(const std::exception& e)
    {
        lg(e.what());
        return ml::ret::fail("[FxPipe::save] couldn't save the data : " + jdata.dump(4) + "\nIn the file " + path + "\nMore infos on the error : " + e.what());
    }
    return ml::ret::success();
}

json FxPipe::_reparent(const json& args)
{
    json _r;	

    auto tasks = this->tasks(this->asList(args, "id"));
    Tasks* tasksList = nullptr;

    if (args.contains("parent"))
    {
        auto parent = this->task(args["parent"].get<std::string>());
        if (parent.success)
            tasksList = &parent.value->subtasks();
    }

#ifdef mydebug
    for (auto task : tasks)
    {
        if(args.contains("parent"))
            lg("reparent task " + task->id() + " to " + args["parent"].get<std::string>());
        else
            lg("reparent task " + task->id() + " to root.");
    }
#endif

    ml::Vec<Task*> reparented;
    if (!tasksList)
        tasksList = &_tasks;
    for (auto task : tasks)
    {
        auto res = tasksList->add(task->id());
        if (res.success)
            reparented.push_back(task);
    }

    _r["data"] = json::object();
    if (args.contains("parent"))
        _r["data"]["parent"] = args["parent"].get<std::string>();
    else 
        _r["data"]["parent"] = "";
    _r["data"]["reparented"] = json::array();
    for (auto task : reparented)
        _r["data"]["reparented"].push_back(task->serialize());
    ipc::success(_r);
    return _r;
}

json FxPipe::_emitTaskEvent(const json& args)
{
    json _r;	
    auto tasks = this->tasks(this->asList(args, "id"));
    for (auto task : tasks)
    {
        //TODO : add custom data could be cool ! 
        task->emitEvent(args["event"].get<std::string>());
    }
    return _r;
}

json FxPipe::_getAllData(const json& args)
{
    json _r;
    _r["data"] = this->serialize();
    ipc::success(_r);
    return _r;
}

json FxPipe::_newProject(const json& args)
{
    json _r;	
    _tasks.clear();
    _categories.clear();
    ipc::success(_r);
    return _r;
}

json FxPipe::_reorderTasks(const json& args)
{
    json _r;
    Tasks* tasks = &_tasks;
    if (args.contains("parent"))
    {
        auto parent = this->task(args["parent"].get<std::string>());
        if (!parent.success)
        {
            ipc::error(_r, parent.message);
            return _r;
        }
        tasks = &parent.value->subtasks();
    }

    auto succ = tasks->reorder(args["tasks"].get<std::vector<std::string>>());
    if (!succ.success)
    {
        ipc::error(_r, succ.message);
        return _r;
    }

    ipc::success(_r);
    return _r;
}

json FxPipe::_setProjectSettings(const json& args)
{
    json _r;
    _projectSettings.deserialize(args);

    ipc::success(_r);
    _r["data"] = _projectSettings.serialize();
    return _r;
}

json FxPipe::_getProjectSettings(const json& args)
{
    json _r;
    _r["data"] = _projectSettings.serialize();
    ipc::success(_r);
    return _r;
}

std::unique_ptr<Task>& FxPipe::taskUniqPtr(const std::string&id, Tasks* parent)
{
    if(!parent)
        parent = &_tasks;

    for (auto& task : parent->tasks())
    {
        if (task->id() == id)
            return task;
        auto &ctask = this->taskUniqPtr(id, &task->subtasks());
        if (!this->isTaskNull(ctask))
            return ctask;
    }
    return _nullTask;
}

std::unique_ptr<Task>& FxPipe::taskUniqPtr(const std::string&id, Task* parent)
{
    return this->taskUniqPtr(id, &parent->subtasks());
}

std::unique_ptr<Task>& FxPipe::taskUniqPtr(const std::string&id, const std::string& parentId)
{
    auto tasks = this->task<Task>(parentId);
    if (!tasks.success)
        return _nullTask;

    return this->taskUniqPtr(id, tasks.value);
}

bool FxPipe::isTaskNull(std::unique_ptr<Task>& task) const
{
    return task == nullptr || task == _nullTask;
}

ml::Vec<Task*> FxPipe::tasks(const ml::Vec<std::string>& ids)
{
    ml::Vec<Task*> tasks;	
    for (auto& id : ids)
    {
        auto t = this->task(id);
        if (t.success)
            tasks.push_back(t.value);
    }
    return tasks;
}

ml::Vec<std::string> FxPipe::asList(const json& data, const std::string& key)
{
    ml::Vec<std::string> ids;	
    if (!data.contains(key))
        return ids;

    if (data[key].is_string())
    {
        try
        {
            ids.push_back(data[key].get<std::string>());
        }
        catch(const std::exception& e)
        {
            return ids;
        }
    }
    else if (data[key].is_array())
    {
        for (auto& id : data[key])
        {
            try
            {
                ids.push_back(id.get<std::string>());
            }
            catch(const std::exception& e)
            {
                continue;
            }
        }
    }
    return ids;
}

json FxPipe::_createNewVersion(const json& args)
{
    json _r;	
    std::string notes = "";
    if (args.contains("notes"))
        notes = args["notes"].get<std::string>();
    this->versions().createNew(this->serialize(), notes);
    return _r;
}

json FxPipe::_listVersions(const json& args)
{
    json _r;	
    int quantity = 10;
    if (args.contains("quantity"))
        quantity = args["quantity"].get<int>();

    _versions.read(quantity);
    _r["data"] = _versions.serialize();
    ipc::success(_r);
    return _r;
}

json FxPipe::_getStats(const json& args)
{
    json _r;	
    int quantity = 1;
    if (args.contains("quantity"))
        quantity = args["quantity"].get<int>();

    auto res = _stats.read(quantity);
    if (!res.success)
    {
        ipc::error(_r, res.message);
        return _r;
    }
    _r["data"] = _stats.data();
    ipc::success(_r);

    return _r;
}

json FxPipe::_currentStatsDone(const json& args)
{
    json _r;	
    auto res = _stats.read(1);

    if (!res.success)
    {
        ipc::error(_r, res.message);
        return _r;
    }

    std::string _commit;
    for (const auto& stat : _stats.data().items())
    {
        auto arr = stat.value();
        for (const auto &item : arr)
        {
            std::string name;
            if (item.contains("name"))
                name = item["name"].get<std::string>();
            std::string id;
            if (item.contains("id"))
                id = item["id"].get<std::string>();
            _commit += " -> Task " + name + " (" + id + ") status changed : ";

            Task::Status status; 
            if (item.contains("status"))
                status = item["status"].get<Task::Status>();

            if (status == Task::DONE)
                _commit += "Done";
            else if (status == Task::STARTED)
                _commit += "Started";
            else
                _commit += "Changed";
            _commit += "\n";
        }
    }
    _r["data"] = _commit;
    ipc::success(_r);

    return _r;
}

json FxPipe::_search(const json& args)
{
    json _r;	
    _r["data"] = json::array();
    std::string query = args["query"];
    query = str::clean(query, true);
    query = str::replace(query, "-", "");
    query = str::replace(query, "_", "");

    if(query.empty())
        return _r;

    for (auto& task : this->allTasks())
    {
        if (task->searchString().find(query) != std::string::npos)
            _r["data"].push_back(task->serialize());
    }

    ipc::success(_r);
    return _r;
}

ml::Vec<Task*> FxPipe::allTasks()
{
    return _tasks.allTasks();
}
