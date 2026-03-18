#include "./Task.h"
#include "str.h"
#include "mlTime.h"
#include <boost/process.hpp>
#include "./FxPipe.h"
#include "./Stats.h"

void Task::_init()
{
    _id = str::random(20);
    _creationTime = ml::time::now();
    
    _setEvents();
}

void Task::_setEvents()
{
    auto onstatus = [this]	
    {
        FxPipe::get()->stats().saveTaskStatusChange(this);
    };
    
    _events.add("status-changed", onstatus);
}

Task::Task(): _color(.1, .1, .1), _subtasks(this)
{
    _init();
}

Task::Task(const json& data): _color(.1, .1, .1),_subtasks(this)
{
    _init();
    this->deserialize(data);	
}

json Task::serialize() const
{
    json _r;
    _r["id"] = _id;
    _r["category"] = _category;
    _r["name"] = _name;
    _r["description"] = _description;

    _r["deadline"] = _deadline;
    _r["color"] = _color.serialize();

    _r["status"] = _status;
    _r["priority"] = _priority;

    _r["price"] = _price.serialize();

    _r["files"] = json::array();
    for (auto& file : _files)
        _r["files"].push_back(file);

    _r["version"] = json::array();
    for (auto& version : _version)
        _r["version"].push_back(version.serialize());

    _r["addFilesToCmds"] = _addFilesToCmds;
    _r["archived"] = _archived;

    _r["oneventsCmds"] = json::object();
    for (auto& oneventsCmd : _oneventsCmds)
    {
        _r["oneventsCmds"][oneventsCmd.first] = json::array();
        for (auto & cmd : oneventsCmd.second)
            _r["oneventsCmds"][oneventsCmd.first].push_back(cmd);
    }

    _r["subtasks"] = _subtasks.serialize();
    _r["creationTime"] = _creationTime;
    _r["parent"] = "";
    if(_parent && _parent->parent())
        _r["parent"] = _parent->parent()->id();

    _r["progress"] = this->progress();

    return _r;
}

void Task::deserialize(const json& data, bool emitEventsIfChanged)
{
    std::vector<std::string> toEmit;
    if (data.contains("id"))
    {
        if (emitEventsIfChanged && _id != data["id"])
            toEmit.push_back("id");
        _id = data["id"];
    }
    if (data.contains("category"))
    {
        if (emitEventsIfChanged && _category != data["category"])
            toEmit.push_back("category");
        _category = data["category"];
    }
    if (data.contains("name"))
    {
        if (emitEventsIfChanged && _name != data["name"])
            toEmit.push_back("name");
        _name = data["name"];
    }
    if (data.contains("description"))
    {
        if (emitEventsIfChanged && _description != data["description"])
            toEmit.push_back("description");
        _description = data["description"];
    }

    if (data.contains("deadline"))
    {
        if (emitEventsIfChanged && _deadline != data["deadline"])
            toEmit.push_back("deadline");

        if(data["deadline"].is_number())
            _deadline = data["deadline"];
        else if (data["deadline"].is_string())
        {
            try
            {
                _deadline = ml::time::fromString(data["deadline"],  "%Y-%m-%d %H:%M:%S");
            }
            catch(const std::exception& e)
            {
                _deadline = 0;
                lg(e.what());
            }
        }
    }
    if (data.contains("color"))
    {
        Color<float> color;
        color.deserialize(data["color"]);
        if (emitEventsIfChanged && color != _color)
            toEmit.push_back("color");
        _color.deserialize(data["color"]);
    }

    if (data.contains("status"))
    {
        if (emitEventsIfChanged && _status != data["status"])
        {
            toEmit.push_back("status");
        }
        _status = data["status"];

        if (emitEventsIfChanged)
            _events.emit("status-changed");
    }

    if (data.contains("priority"))
    {
        if (emitEventsIfChanged && _priority != data["priority"])
            toEmit.push_back("priority");
        _priority = data["priority"];
    }

    if (data.contains("price"))
    {
        Price p;
        p.deserialize(data["price"]);
        if (emitEventsIfChanged && _price != p)
            toEmit.push_back("price");
        _price.deserialize(data["price"]);
    }

    if (data.contains("files"))
    {
        //TODO : check if emitEventsIfChanged
        _files.clear();
        for(auto& file : data["files"])
            _files.push_back(file);
    }

    if (data.contains("version"))
    {
        //TODO : check if emitEventsIfChanged
        _version.clear();
        for(auto& version : data["version"])
            _version.push_back(Version(version));
    }

    if (data.contains("addFilesToCmds"))
    {
        if (emitEventsIfChanged && _addFilesToCmds != data["addFilesToCmds"])
            toEmit.push_back("addFilesToCmds");
        _addFilesToCmds = data["addFilesToCmds"];
    }

    if (data.contains("oneventsCmds"))
    {
        //TODO : check if emitEventsIfChanged
        _oneventsCmds.clear();
        for(auto& oneventsCmd : data["oneventsCmds"].items())
        {
            _oneventsCmds[oneventsCmd.key()] = {};
            for(auto& cmd : oneventsCmd.value())
                _oneventsCmds[oneventsCmd.key()].push_back(cmd);
        }
    }

    if (data.contains("subtasks"))
        _subtasks.deserialize(data["subtasks"]);

    if (data.contains("archived"))
    {
        if (emitEventsIfChanged && _archived != data["archived"])
            toEmit.push_back("archived");
        _archived = data["archived"];
    }

    if (data.contains("creationTime"))
    {
        if (emitEventsIfChanged && _creationTime != data["creationTime"])
            toEmit.push_back("creationTime");
        _creationTime = data["creationTime"];
    }

    if (data.contains("parent"))
    {
        std::string pid = data["parent"];
        if (pid.empty())
        {
            _parent = &FxPipe::get()->tasks();
        }
        else 
        {
            auto ret = FxPipe::get()->task(pid);
            if (ret.success)
            {
                _parent = &ret.value->subtasks();
            }
            else
            {
                _parent = &FxPipe::get()->tasks();
            }
        }
    }

    if (emitEventsIfChanged)
    {
        for (auto& eventname : toEmit)
            this->emitEvent(eventname);
    }
}

float Task::progress() const
{
    if (_status == DONE ) 	
        return 1;

    if (_subtasks.tasks().empty())
        return 0;
    else 
    {
        float r = 0;
        for (auto& subtask : _subtasks.tasks())
            r += subtask->progress();
        return r / (float)_subtasks.tasks().size();
    }

    return 0;
}

//make the boost::process::childs in children live after this program without waiting.
void Task::emitEvent(const std::string& eventname) const
{
    for (const auto& ev : _onevents)    
        ev(eventname);

    if (_oneventsCmds.find(eventname) == _oneventsCmds.end())
        return;

    ml::Vec<boost::process::child> children;
    try
    {
        const auto& cmds = _oneventsCmds.at(eventname);
        for (auto& cmd : cmds)
        {
            boost::process::child child(cmd, boost::process::std_out > boost::process::null, boost::process::std_err > boost::process::null);
            child.detach();
        }
    }
    catch(const std::exception& e)
    {
        lg("Process launch error from the event " << eventname << ": ");
        lg(e.what());
    }
}

void Task::open() const
{
    this->emitEvent("open");
}

std::string Task::parentId() const
{
    auto p = this->parentTask();
    if(!p)
        return "";
    return p->id();
}

Task* Task::parentTask() const
{
    if (!_parent)
        return nullptr;
    if (_parent == &FxPipe::get()->tasks())
        return nullptr;
    return _parent->parent();
}

std::string Task::searchString()
{
    std::string s;	
    s = _name + "\n" + _description + "\n" + _category + "\n" + _id;
    s = str::clean(s, true);
    s = str::replace(s, "-", "");
    s = str::replace(s, "_", "");
    return s;
}
