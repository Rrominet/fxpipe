#include "./TaskView.h"
#include "./FxPipe.h"

#include "mlgui.2/src/Box.h"
#include "mlgui.2/src/Box.hpp"

#include "./Task.h"
#include "mlgui.2/src/enums.h"
#include "mlgui.2/src/GuiBackendCommand.h"

TaskView::TaskView(ml::Box* parent, Task* taskParent): _taskParent(taskParent)
{
    _box = parent->createBox().get();	
    _box->setHExpand(true);
    _box->setVExpand(true);

    _box->addCssClass("taskview");

    _setEvents();
}

TaskView::~TaskView()
{
	
}

Task* TaskView::createTask(const json& data)
{
    auto t = std::make_unique<Task>(_box, data, this);	
    _tasks.push_back(std::move(t));
    _box->show();
    return _tasks.back().get();
}

bool TaskView::removeTask(Task* task)
{
    if (!task)
        return false;

    fxpipe::get()->removeFromSelection(task);
    if (fxpipe::get()->activeTask() == task)
        fxpipe::get()->setActiveTask(nullptr);

    for (auto& t : _tasks)
    {
        if (t.get() == task)
        {
            _box->removeChild(t->box());
            _tasks.remove(t);
            return true;
        }
    }

    if (_tasks.size() == 0)
        _box->hide();

    return false;
}

bool TaskView::removeTask(unsigned int index)
{
    if(index < _tasks.size())
        return this->removeTask(_tasks[index].get());
    return false;	
}

void TaskView::addCssClass(const std::string& cls)
{
    _box->addCssClass(cls);	
}

void TaskView::removeCssClass(const std::string& cls)
{
    _box->removeCssClass(cls);	
}

bool TaskView::contains(Task* task)
{
    for (auto& t : _tasks)
    {
        if (t.get() == task)
            return true;
    }
    return false;
}

void TaskView::clear()
{
    for (auto& t : _tasks)
        _box->removeChild(t->box());
    _tasks.clear();	
    _box->hide();
}

void TaskView::_setEvents()
{
    _box->addEventListener(ml::MOUSE_ENTER, [this](ml::EventInfos& e){fxpipe::get()->setActiveTaskView(this);});

//     _box->addEventListener(ml::DRAG_START, [this](ml::EventInfos& e){_onDragStart(e);});
//     _box->addEventListener(ml::DRAG_END, [this](ml::EventInfos& e){_onDragEnd(e);});
//     _box->addEventListener(ml::DRAGGING, [this](ml::EventInfos& e){_onDragging(e);});
}

void TaskView::_onDragStart(ml::EventInfos& e)
{
    lg("Drag start !");
//     if (!fxpipe::get()->activeTask())	
//         return;
//     if (fxpipe::get()->draggedTaskView())
//         return;
// 
//     fxpipe::get()->setDraggedTaskView(this);
//     _drag.canDrag = true;
//     _drag.draggedTask = fxpipe::get()->activeTask();
//     _drag.draggedTask->box()->addCssClass("dragged");
//     _box->setCursor("grabbing");
// 
//     if (e.shift)
//         _drag.action = REPARENT;
//     else 
//         _drag.action = SWAP;
// 
//     lg("Dragging task : " << _drag.draggedTask->id() << " (name : \"" << _drag.draggedTask->data()["name"].get<std::string>() << "\")");
}

void TaskView::_onDragEnd(ml::EventInfos& e)
{
    lg("Drag end !");
//     if (!_drag.canDrag)
//         return;
//     if (_drag.draggedTask)
//         _drag.draggedTask->box()->removeCssClass("dragged");
// 
//     _box->setCursor("default");
// 
//     json data;
//     if (_drag.action == REPARENT)
//     {
//         data["parent"] = _drag.newParentTask->id();
//         data["id"] = _drag.draggedTask->id();
// 
//         auto nparent = _drag.newParentTask;
//         auto cb = [this, nparent](const json& res)
//         {
//             if (!res.contains("data"))
//                 return;
//             auto ntaskdata = res["data"];
//             nparent->tasksView()->createTask(ntaskdata);
// 
//             for (const auto& t : _tasks)
//                 t->removeCssClass("new-parent");
//         };
// 
//         fxpipe::get()->reparentTaskCmd()->addCallback(cb, true);
//         fxpipe::get()->reparentTaskCmd()->execJson(data);
//         this->removeTask(_drag.draggedTask);
//     }
//     else if (_drag.action == SWAP)
//     {
//         if (_taskParent)
//             data["parent"] = _taskParent->id();
//         data["tasks"] = json::array();
//         for (const auto& t : _tasks)
//             data["tasks"].push_back(t->id());
// 
//         fxpipe::get()->reorderTasksCmd()->execJson(data);
//     }
// 
//     fxpipe::get()->setDraggedTaskView(nullptr);
//     _drag.draggedTask = nullptr;
//     _drag.canDrag = false;
//     _drag.newParentTask = nullptr;
}

bool TaskView::swapTasks(Task* t1,Task* t2)
{
    if (t1 == t2)	
        return false;
    int idx1 = -1;
    int idx2 = -1;
    for (auto& t : _tasks)
    {
        if(t.get() == t1)
            idx1 = _tasks.find(t);
        if(t.get() == t2)
            idx2 = _tasks.find(t);
    }

    if (idx1 == -1 || idx2 == -1)
        return false;

    _box->swap(t1->box(), t2->box());
    _tasks.swap(idx1, idx2); 
    return true;
}

void TaskView::_onDragging(ml::EventInfos& e)
{
    lg("Dragging..");
//     if (!_drag.canDrag)
//         return;
// 
//     for (const auto& t : _tasks)
//     {
//         if (t.get() == _drag.draggedTask)
//             continue;
// 
//         if (t->box()->isInside(e.x, e.y))
//         {
//             if (_drag.action == REPARENT)
//             {
//                 _drag.newParentTask = t.get();
//                 t->addCssClass("new-parent");
//                 fxpipe::get()->main()->setInfos("Move in : \"" + t->data()["name"].get<std::string>() + "\"");
//             }
//             else if(_drag.action == SWAP)
//                 this->swapTasks(_drag.draggedTask, t.get());
//         }
//         else 
//             t->removeCssClass("new-parent");
//     }
}

bool TaskView::moveTask(Task* task,int movement)
{
    if (!task || movement == 0)
        return false;
    
    if (!this->contains(task))
        return false;
    
    int currentIdx = -1;
    for (auto& t : _tasks)
    {
        if (t.get() == task)
        {
            currentIdx = _tasks.find(t);
            break;
        }
    }
    
    if (currentIdx == -1)
        return false;
    
    int targetIdx = currentIdx + movement;
    
    if (targetIdx < 0 || targetIdx >= (int)_tasks.size())
        return false;
    
    bool worked = false;
    if (movement > 0)
    {
        for (int i = 0; i < movement; i++)
        {
            int idx = currentIdx + i;
            if (idx + 1 < (int)_tasks.size())
                worked = this->swapTasks(_tasks[idx].get(), _tasks[idx + 1].get());
        }
    }
    else
    {
        for (int i = 0; i < -movement; i++)
        {
            int idx = currentIdx - i;
            if (idx - 1 >= 0)
                worked = this->swapTasks(_tasks[idx].get(), _tasks[idx - 1].get());
        }
    }

    return worked;
}

bool TaskView::moveSelectedTasks(int movement)
{
    if (movement == 0)
        return false;
    
    auto selected = fxpipe::get()->selectedTasks();
    for (int i = selected.size() - 1; i >= 0; i--)
    {
        if(selected[i]->parentView() != this)
            selected.removeByIndex(i);
    }
    
    if (selected.size() == 0)
        return false;
    
    // Get indices of selected tasks
    ml::Vec<int> selectedIndices;
    for (auto task : selected)
    {
        for (auto& t : _tasks)
        {
            if (t.get() == task)
            {
                selectedIndices.push_back(_tasks.find(t));
                break;
            }
        }
    }
    
    // Sort indices
    selectedIndices.sort([](int a, int b) { return a < b; });
    
    // Check if movement is possible
    if (movement < 0)
    {
        // Moving up - check first selected task
        int firstIdx = selectedIndices[0];
        if (firstIdx + movement < 0)
            return false;
    }
    else
    {
        // Moving down - check last selected task
        int lastIdx = selectedIndices[selectedIndices.size() - 1];
        if (lastIdx + movement >= (int)_tasks.size())
            return false;
    }
    
    // Perform the movement
    bool worked = true;
    if (movement > 0)
    {
        // Moving down - process from last to first
        for (int i = selectedIndices.size() - 1; i >= 0; i--)
        {
            int currentIdx = selectedIndices[i];
            for (int j = 0; j < movement; j++)
            {
                if (currentIdx + j + 1 < (int)_tasks.size())
                {
                    if (!this->swapTasks(_tasks[currentIdx + j].get(), _tasks[currentIdx + j + 1].get()))
                    {
                        worked = false;
                        break;
                    }
                }
            }
            if (!worked)
                break;
        }
    }
    else
    {
        // Moving up - process from first to last
        for (int i = 0; i < (int)selectedIndices.size(); i++)
        {
            int currentIdx = selectedIndices[i];
            for (int j = 0; j < -movement; j++)
            {
                if (currentIdx - j - 1 >= 0)
                {
                    if (!this->swapTasks(_tasks[currentIdx - j].get(), _tasks[currentIdx - j - 1].get()))
                    {
                        worked = false;
                        break;
                    }
                }
            }
            if (!worked)
                break;
        }
    }

    fxpipe::get()->reorderTasksCmd()->exec();
    return worked;
}

