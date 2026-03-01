#pragma once
#include "mlgui.2/src/EventInfos.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Ret.h"
#include "vec.h"

namespace ml
{
    class Box;
    class Label;
}

class TaskView;
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

        Task(ml::Box* parent, const json& data, TaskView* parentView=nullptr);
        virtual ~Task() = default;

        void draw(ml::Box* parent);
        void setStatus(Status status, bool sendToBackend=false);
        void setPriority(Priority priority);

        Status status() const;
        Priority priority() const;

        void toggleBody();

        void addCssClass(const std::string& cls);
        void removeCssClass(const std::string& cls);

        void incrementStatus(int inc, bool sendToBackend=false);
        void incrementPriority(int inc);

        static std::string statusAsString(Status status);
        static std::string priorityAsString(Priority priority);

        static Status stringAsStatus(const std::string& s);
        static Priority stringAsPriority(const std::string& s);

        bool isChild(Task* task);
        ml::Ret<Task*> parent();

        TaskView* tasksView()const {return _subtasks.get();}

        std::string id()const {if (!_data.contains("id")) return ""; 
            return _data["id"].get<std::string>();}

        //this is reccursive
        ml::Ret<Task*> child(const std::string& id);
        
        //it will also update the active gui if possible.
        void update(const json& data = {});

        //between 0 and 1
        void setProgress(float progress);

        void open();
        ml::Vec<Task*> children(bool reccursive=false);

        void setVisible(bool visible);
        bool visible();
        bool selected();

        bool bodyShown() const;
        void showBody();
        void hideBody();
        void remove();
        
        //inex in his list of subtasks
        int index() const;

    protected : 
        json _data; //bp cg

        ml::Box* _box = nullptr; //bp cg
        ml::Box* _header = nullptr;
        ml::Label* _name = nullptr;
        ml::Label* _status = nullptr;
        ml::Label* _priority = nullptr;

        ml::Box* _body = nullptr;
        ml::Label* _description = nullptr;

        Status _statusVal = NOT_STARTED;
        Priority _priorityVal = LOW;

        std::unique_ptr<TaskView> _subtasks;
        TaskView* _parentView = nullptr; //bp cg

        void _setEvents();

        void _onStatusWheel(ml::EventInfos& e);
        void _onPriorityWheel(ml::EventInfos& e);
        void _onLeftUp(ml::EventInfos& e);
        //std::function<void (const json& res)> _updateCb;

    public : 
#include "./Task_gen.h"
};
