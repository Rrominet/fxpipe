#include "./TaskSettingsWindow.h"

#include "mlgui.2/src/Scrollable.hpp"
#include "mlgui.2/src/enums.h"
#include "mlgui.2/src/Collapsable.h"
#include "./TaskEventsCommands.h"

#define CONSTRUCT_PROPS \
    _type("Type", "Task", "The type of task", {"Task"}), \
    _name("Name", "", "Name of the task"), \
    _description("Description", "", "A detailed description of the task to do if needed."), \
    _category("Category", "None", "The category of the task", {"None"}), \
    _status("Status", "Not Started", "The status of the task", {"Not Started", "Started", "Need Review", "Need Retakes", "Done"}), \
    _priority("Priority", "Low", "The priority of the task", {"Low", "Medium", "High", "This month", "This week", "Today", "Urgent"}), \
    _archived("Archived", false), \
    _deadline("Deadline")

TaskSettingsWindow::TaskSettingsWindow(ml::App* app) : ml::Window(app), CONSTRUCT_PROPS
{
	
}

 TaskSettingsWindow::TaskSettingsWindow(ml::App* app,ml::Window* parent) : ml::Window(app, parent), CONSTRUCT_PROPS
{
	
}

 TaskSettingsWindow::~TaskSettingsWindow()
{
	
}

void TaskSettingsWindow::init()
{
    ml::Window::init();	
    this->setSize(600, 750);
    this->setTitle("Task Settings");

    _main->appendEnumProp(&_type);
    _parentName = _main->createLabel("Parent Task : None").get();
    _parentName->addCssClass("parent-name");
    _parentName->setHAlign(ml::RIGHT);
    _parentName->setHExpand(true);

    _main->appendProp(&_name);
    _main->appendProp(&_description, ml::MULTILINE_ENTRY);
    _main->appendEnumProp(&_category);
    _main->createSeparator();
    _main->appendEnumProp(&_status);
    _main->appendEnumProp(&_priority);
    _main->createSeparator();
    _main->appendProp(&_deadline, ml::DATE_TIME);
    _eventsCmdsCollapsable = _main->createComposedWidget<ml::Collapsable>(&_main->content(), "Events & Commands", true).get();
    _taskEventsCommands = std::make_unique<TaskEventsCommands>(_eventsCmdsCollapsable->body().get());
    _main->createSeparator();
    _main->appendProp(&_archived, ml::CHECKBOX);
    _main->createSeparator();
    _cmdButton = _main->appendCommand("create-or-modify-task", "Create Task").get();

    this->addKeybind("ctrl enter", [this]{
                ml::app()->cmds().exec("create-or-modify-task");
                this->hide();
                return true;
            });
}

void TaskSettingsWindow::setFromTask(Task* task)
{
    this->reset();
    _id = task->id();
    _cmdButton->setText("Modify Task");
    this->setMode(MODIFY);
    auto& data = task->data();
    
    if (data.contains("name"))
        _name.set(data["name"].get<std::string>());
    if (data.contains("description"))
        _description.set(data["description"].get<std::string>());
    if (data.contains("category"))
        _category.set(data["category"].get<std::string>());
    if (data.contains("status"))
        _status.set(Task::statusAsString(data["status"].get<Task::Status>()));
    if (data.contains("priority"))
        _priority.set(Task::priorityAsString(data["priority"].get<Task::Priority>()));
    if (data.contains("deadline"))
        _deadline.set(ml::time::asStringReverse(data["deadline"].get<unsigned long>()));
    if (data.contains("archived"))
        _archived.set(data["archived"].get<bool>());
    if (data.contains("oneventsCmds"))
        _taskEventsCommands->deserialize(data["oneventsCmds"]);
    if (data.contains("parent")) {
        std::string parentId = data["parent"].get<std::string>();
        this->setParent(parentId, "Parent");
    }
}

void TaskSettingsWindow::setMode(Mode mode)
{
    switch (mode)
    {
        case CREATE : 
            _cmdButton->setText("Create Task");
            break;
        case MODIFY :
            _cmdButton->setText("Modify Task");
            break;
    }	
    _mode = mode;
}

void TaskSettingsWindow::reset()
{
    _id = "";
    _parentId = "";
    _cmdButton->setText("Create Task");
    _parentName->setText("Parent Task : None");
    _name.set(_S"");
    _description.set(_S"");
    _category.set(_S"None");
    _status.set(_S"Not Started");
    _priority.set(_S"Low");
    _archived.set(false);
    _deadline.set(_S"");
    _taskEventsCommands->clear();
}

json TaskSettingsWindow::data()
{
    json _r;	
    if (!_id.empty())
        _r["id"] = this->id();
    if (!_parentId.empty())
        _r["parent"] = _parentId;
    _r["name"] = _name.value();
    _r["description"] = _description.value();
    _r["category"] = _category.value();
    _r["status"] = this->status();
    _r["priority"] = this->priority();
    _r["deadline"] = _deadline.value();
    _r["archived"] = _archived.value();
    _r["oneventsCmds"] = _taskEventsCommands->serialize();

    lg("TaskSettingsWindow::data\n" << _r.dump(4));
    return _r;
}

Task::Status TaskSettingsWindow::status()
{
    return Task::stringAsStatus(_status.value());	
}

Task::Priority TaskSettingsWindow::priority()
{
    return Task::stringAsPriority(_priority.value());	
}

void TaskSettingsWindow::setParent(const std::string& id,const std::string& name)
{
    _parentId = id;	
    _parentName->setText("Parent Task : " + name);
}
