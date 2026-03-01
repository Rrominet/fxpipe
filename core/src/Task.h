#pragma once
#include "str.h"
#include "Color.h"
#include "vec.h"
#include "./Price.h"
#include "./Version.h"
#include <nlohmann/json.hpp>
#include <unordered_map>
#include "./Tasks.h"
#include "Events.h"

using json = nlohmann::json;

class Task
{
    public : 
        enum Status
        {
            NOT_STARTED,
            STARTED,
            NEED_REVIEW,
            NEED_RETAKES,
            DONE,
        };

        enum Priority
        {
            LOW,
            MEDIUM,
            HIGH,
            THIS_MONTH,
            THIS_WEEK,
            TODAY,
            URGENT,
        };

        Task();
        Task(const json& data);
        ~Task() = default;

        json serialize() const;
        void deserialize(const json& data, bool emitEventsIfChanged = false);

        //between 0 and 1;
        float progress() const;

        void emitEvent(const std::string& eventname) const; 
        void open() const;

        std::string parentId() const;
        Task* parentTask() const;

    protected: 
        std::string _id; //bp cg
        
        //used for serialize and deserialize subChild class Task
        std::string _type = "Task"; //cb cg
        std::string _category;//bp cgs
        std::string _name;//bp cgs
        std::string _description;//bp cgs

        bool _archived = false;//bp cgs;

        int64_t _creationTime;//bp cgs
        int64_t _deadline;//bp cgs
        Color<float> _color;//bp cgs

        Status _status;//bp cgs
        Priority _priority;//bp cgs

        Price _price;//bp cgs

        ml::Vec<std::string> _files;//bp cg
        ml::Vec<Version> _version;  //bp cg

        bool _addFilesToCmds = false; //bp cgs

        // the events could be anything here !
        // open, close, and any property changed like priority-changed, etc.
        // the events are not sended by the gui but directly call in server call.
        // the gui only all the server registered functions
        ml::Vec<std::function<void(const std::string& event)>> _onevents; //bp cgs
        std::unordered_map<std::string, ml::Vec<std::string>> _oneventsCmds; //cgs

        Tasks _subtasks; //bp cg
        Tasks* _parent = nullptr; //bp cgs

        //used to being bind to the stats saving system.
        ml::Events _events; //bp cg

        void _init();
        void _setEvents();

    public :
#include "./Task_gen.h"
};
