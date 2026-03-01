#pragma once
#include "str.h"
#include "vec.h"
#include "mlgui.2/src/EventInfos.h"

namespace ml
{
    class Box;
}

class Task;

enum DragAction
{
    SWAP,
    REPARENT
};

struct TaskViewDrag
{
    Task* draggedTask = nullptr;
    bool canDrag = false;
    Task* newParentTask = nullptr;
    DragAction action = SWAP;
};

class TaskView
{
    public : 
        TaskView(ml::Box* parent, Task* taskParent=nullptr);
        virtual ~TaskView();
        
        Task* createTask(const json& data);
        bool removeTask(Task* task);
        bool removeTask(unsigned int index);

        void addCssClass(const std::string& cls);
        void removeCssClass(const std::string& cls);

        bool contains(Task* task);
        void clear();

        bool swapTasks(Task* t1, Task* t2);
        bool moveTask(Task* task, int movement);
        bool moveSelectedTasks(int movement);

    protected : 
        Task* _taskParent = nullptr; //bp cg
        ml::Box* _box = nullptr; //bp cg
        ml::Vec<std::unique_ptr<Task>> _tasks; //bp cg

        void _setEvents();
        void _onDragStart(ml::EventInfos& e);
        void _onDragEnd(ml::EventInfos& e);
        void _onDragging(ml::EventInfos& e);
        
        TaskViewDrag _drag; //bp cg

    public :
#include "./TaskView_gen.h"
};
