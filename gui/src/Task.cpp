#include "./Task.h"

#include "Ret.h"
#include "mlgui.2/src/EventInfos.h"
#include "mlgui.2/src/Label.h"
#include "mlgui.2/src/Box.h"
#include "mlgui.2/src/Box.hpp"
#include "mlgui.2/src/enums.h"
#include "mlgui.2/src/GuiBackendCommand.h"

#include <unordered_map>

#include "./FxPipe.h"
#include "./TaskView.h"
#include "ipc.h"

std::unordered_map<Task::Status, std::string> status_map = {
    {Task::NOT_STARTED, "Not started"}, 
    {Task::STARTED, "Started"}, 
    {Task::NEED_REVIEW, "Need review"}, 
    {Task::NEED_RETAKES, "Need retakes"}, 
    {Task::DONE, "Done"}, 
};

std::unordered_map<Task::Priority, std::string> status_priorities = {
    {Task::LOW, "Low"}, 
    {Task::MEDIUM, "Medium"}, 
    {Task::HIGH, "High"}, 
    {Task::THIS_MONTH, "This month"}, 
    {Task::THIS_WEEK, "This week"}, 
    {Task::TODAY, "Today"}, 
    {Task::URGENT, "Urgent"},
};

std::string Task::statusAsString(Status status)
{
    return status_map[status];	
}

std::string Task::priorityAsString(Priority priority)
{
    return status_priorities[priority];	
}

Task::Status Task::stringAsStatus(const std::string& s)
{
    for (auto it = status_map.begin(); it != status_map.end(); it++)
    {
        if (it->second == s)
            return it->first;
    }
    return Task::NOT_STARTED;	
}

Task::Priority Task::stringAsPriority(const std::string& s)
{
    for (auto it = status_priorities.begin(); it != status_priorities.end(); it++)
    {
        if (it->second == s)
            return it->first;
    }
    return Task::LOW;
}

Task::Task(ml::Box* parent, const json& data, TaskView* parentView): _parentView(parentView)
{
    _data = data;
//     _updateCb = [this](const json& res)
//     {
//         if (!res.contains("data"))
//             return;
//         this->update(res["data"]);
//     };
    this->draw(parent);
    _setEvents();
}

void Task::draw(ml::Box* parent)
{
    _box = parent->createBox().get();	
    _box->setHExpand(true);
    _box->addCssClass("task");

    _box->setCursor("pointer");

    _header = _box->createBox().get();
    _header->addCssClass("header");
    _header->setHExpand(true);
    _header->setOrient(ml::HORIZONTAL);
    _name = _header->createLabel().get();
    _name->addCssClass("name");

    _status = _header->createLabel().get();
    _status->addCssClass("status");
    _status->setHExpand(true);
    _status->setHAlign(ml::RIGHT);

    _priority = _header->createLabel().get();
    _priority->addCssClass("priority");
    _priority->setHAlign(ml::RIGHT);

    _body = _box->createBox().get();
    _body->addCssClass("body");
    _body->setHExpand(true);
    _body->hide();

    _description = _body->createLabel().get();
    _description->addCssClass("description");
    _description->setHExpand(true);
    _description->setHAlign(ml::RIGHT);
    _description->setTextJustify(ml::RIGHT);

    _subtasks = std::make_unique<TaskView>(_body, this);
    _subtasks->box()->setVExpand(false); //changed
    _subtasks->box()->addCssClass("child");

    _box->setContextMenu("task-context");

    this->update();
}

void Task::update(const json& data)
{
    if (!data.is_null())
        _data = data;
    
    if (_data.contains("name"))
        _name->setText(_data["name"]);
    
    if (_data.contains("status"))
        this->setStatus((Status)_data["status"]);
    
    if (_data.contains("priority"))
        this->setPriority((Priority)_data["priority"]);
    
    if (_data.contains("description"))
        _description->setText(_data["description"]);

    if (_data.contains("progress"))
        this->setProgress(_data["progress"]);
    
    if(_data.contains("subtasks"))
    {
        _subtasks->clear();
        if (_data["subtasks"].size() == 0)
            _subtasks->box()->hide();
        else 
            _subtasks->box()->show();
        for (const auto& t : _data["subtasks"])
            _subtasks->createTask(t);
    }
}

void Task::_setEvents()
{
    _box->addEventListener(ml::LEFT_UP, [this](ml::EventInfos& e){_onLeftUp(e);});	
    _status->addEventListener(ml::WHEEL, [this](ml::EventInfos& e){this->_onStatusWheel(e);});
    _priority->addEventListener(ml::WHEEL, [this](ml::EventInfos& e){this->_onPriorityWheel(e);});
}

void Task::_onLeftUp(ml::EventInfos& e)
{
    e.stopPropagation();
    lg("Left UP for " << this->data()["name"]);
    if(!e.ctrl && !e.shift && !e.alt)
    {
        if (!this->selected() || fxpipe::get()->selectedTasks().size() > 1)
            fxpipe::get()->replaceSelection(this);
        else 
            fxpipe::get()->removeFromSelection(this);
    }
    else if (e.ctrl && !e.shift && !e.alt)
    {
        if (!this->selected())
            fxpipe::get()->addToSelection(this);
        else 
            fxpipe::get()->removeFromSelection(this);
    }
    else if (!e.ctrl && e.shift && !e.alt)
    {
        if (fxpipe::get()->activeTask() && fxpipe::get()->activeTask()->parentView() == this->parentView())
        {
            auto start = fxpipe::get()->activeTask()->index();
            auto end = this->index();

            if (start > end)
                std::swap(start, end);

            for (auto i = start; i <= end; i++)
            {
                auto toadd = this->parentView()->tasks()[i].get();
                if (!toadd->selected() && toadd->visible())
                    fxpipe::get()->addToSelection(toadd, false);
            }
            fxpipe::get()->events().emit("selection-changed");
        }
        else 
        {
            if (!this->selected())
                fxpipe::get()->addToSelection(this);
            else 
                fxpipe::get()->removeFromSelection(this);
        }
    }
}

void Task::_onStatusWheel(ml::EventInfos& e)
{
    if (e.dy < 0)
        this->incrementStatus(-1, true);
    else
        this->incrementStatus(1, true);

}

void Task::_onPriorityWheel(ml::EventInfos& e)
{
    if (e.dy < 0)
        this->incrementPriority(-1);
    else
        this->incrementPriority(1);
    ipc::call(fxpipe::get()->backend(), "create-or-modify-task", _data, 0, true);
}

void Task::toggleBody()
{
    if (_body->isVisible())
        _body->hide();
    else
        _body->show();	
}

Task::Status Task::status() const
{
    return (Status)_data["status"];
}

Task::Priority Task::priority() const
{
    return (Priority)_data["priority"];
}

void Task::setStatus(Status status, bool sendToBackend)
{
    _data["status"] = status;
    _statusVal = status;
    std::string s = status_map[_statusVal];
    _status->setText(s);
    _status->clearCssClasses();
    _status->addCssClass("status");
    _status->addCssClass(str::clean(s, true));

    if (!sendToBackend)
        return;

    ipc::call(fxpipe::get()->backend(), "create-or-modify-task", _data, 0, true);
}

void Task::setPriority(Priority priority)
{
    _data["priority"] = priority;
    _priorityVal = priority;
    std::string s = status_priorities[_priorityVal];
    _priority->setText(s);
    _priority->clearCssClasses();
    _priority->addCssClass("priority");
    _priority->addCssClass(str::clean(s, true));
}

void Task::incrementStatus(int inc, bool sendToBackend)
{
    int current = (int)	_statusVal;
    current += inc;
    if (current < 0)
        current = (int)Task::DONE;
    if (current > (int)Task::DONE)
        current = 0;
    this->setStatus((Status)current, sendToBackend);
}

void Task::incrementPriority(int inc)
{
    int current = (int)	_priorityVal;
    current += inc;
    if (current < 0)
        current = (int)Task::URGENT;
    if (current > (int)Task::URGENT)
        current = 0;
    this->setPriority((Priority)current);
}

void Task::addCssClass(const std::string& cls)
{
    _box->addCssClass(cls);	
}

void Task::removeCssClass(const std::string& cls)
{
    _box->removeCssClass(cls);	
}

bool Task::isChild(Task* task)
{
    return _subtasks->contains(task);
}

ml::Ret<Task*> Task::parent()
{
    if (!_parentView)	
        return ml::ret::fail<Task*>("No parent view");
    if (!_parentView->taskParent())
        return ml::ret::fail<Task*>("No parent Task");
    else 
        return ml::ret::success(_parentView->taskParent());
}

ml::Ret<Task*> Task::child(const std::string& id)
{
    for (auto& t : _subtasks->tasks())
    {
        if (t->id() == id)
            return ml::ret::success(t.get());
        else 
        {
            auto res = t->child(id);
            if (res.success)
                return res;
        }
    }
    return ml::ret::fail<Task*>("Task not found");	
}

void Task::setProgress(float progress)
{
    auto cssPgr = progress * 100;	
    std::string donevar = "var(--done-color)";
    std::string bgvar = "var(--list-background-color)";
    if (_parentView && _parentView->taskParent())
    {
        bgvar = "var(--subdone-color)";
        bgvar = "var(--subtask-color)";
    }
    std::string css = "linear-gradient(90deg, " + donevar + " " + std::to_string(cssPgr) + "%, " + bgvar + " " + std::to_string(cssPgr) + "%)";
    lg(css);
    _box->addCss("background", css);
}

void Task::open()
{
    fxpipe::get()->setActiveTask(this);
    fxpipe::get()->openTaskCmd()->exec();
}

ml::Vec<Task*> Task::children(bool reccursive)
{
    ml::Vec<Task*> tasks;	
    if (!_subtasks)
        return tasks;
    for (const auto& c : _subtasks->tasks())
    {
        tasks.push(c.get());
        if (reccursive)
        {
            auto ctsks = c->children(true);
            for (const auto c2 : ctsks)
                tasks.push(c2);
        }
    }

    return tasks;
}

void Task::setVisible(bool visible)
{
    _box->setVisible(visible);
}

bool Task::visible()
{
    return _box->visible();	
}

bool Task::selected()
{
    return fxpipe::get()->selectedTasks().contains(this);	
}

bool Task::bodyShown() const
{
    return _body->visible();	
}

void Task::showBody()
{
    _body->show();	
}

void Task::hideBody()
{
    _body->hide();	
}

void Task::remove()
{
    _parentView->removeTask(this);
}

int Task::index() const
{
    for (int i = 0; i < _parentView->tasks().size(); i++)
    {
        auto& t = _parentView->tasks()[i];
        if (t.get() == this)
            return i;
    }
    return -1;
}
