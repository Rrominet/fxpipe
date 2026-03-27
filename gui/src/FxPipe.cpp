#include "./FxPipe.h"
#include "./MainWindow.h"
#include "Ret.h"
#include "debug.h"
#include "enums.h"
#include "mlgui.2/src/App.hpp"
#include "mlgui.2/src/GuiBackendCommand.h"
#include "./Task.h"
#include "./TaskView.h"
#include "./TaskSettingsWindow.h"
#include "./ProjectSettingsWindow.h"
#include "./NewVersionWindow.h"
#include "storage.h"
#include "./enums.h"
#include "./FxPipeCommand.h"
#include "./OpenRecentWindow.h"
#include "./GoalsWindow.h"

namespace fxpipe
{
    FxPipe* _fxpipe = nullptr;
}

FxPipe::FxPipe(int argc,char *argv[]) : ml::App(argc,argv) 
{
    fxpipe::_fxpipe = this;	
    this->startBackendProcess();
    this->createCommands();
    this->createWindows();
    this->setEvents();
    storage::init();
    db::setLogFile(files::execDir() + files::sep() + "fxpipe.log");

    auto later = [this]
    {
        this->addCss("fxpipe.css");
    };

    this->setTimeout(later, 16);
    this->openLastProject();

    this->setAbout("fxpipe aim to be a project tracker that is easily extensible and easy to use for the artists, developers, production managers and clients.\n\nIt's written in C++ and based on the fxos-gui library.\nIt uses a server-client architecture approach that let you bind any other gui you want to the server process.\n\nMore infos on https://fxos.com/fxpipe.\nWritten by Romain Gilliot from Motion Live.\n(part of the fxos ecosystem)");

    this->initGoals();
}

void FxPipe::setEvents()
{
    _events.add("active-task-changed", [this]{_onActiveTaskChanged();});
    _events.add("selection-changed", [this]{_onSelectionChanged();});
}

void FxPipe::createCommands()
{
//  auto cmd = _cmds.createCommand<ml::GuiCommand>("Create Command", "create-cmd");
//  cmd->aliases().concat({"ccmd", "ncmd"});
//  cmd->setHelp("Create a new command to execute...");
//  cmd->setKeybind("ctrl n");
//  cmd->setExec([this](const std::any&){auto w = this->createWindow<CommandWindow>(_main); w->show();});


    auto cmd = _cmds.createCommand<ml::GuiCommand>("Create Task", "create-task");
    cmd->setHelp("Open the Window to create a new ask...");
    cmd->setKeybind("alt a");
    cmd->setExec([this](const std::any&){this->showTaskSettingsForCreation();});

    cmd = _cmds.createCommand<ml::GuiCommand>("Edit Task", "modify-task");
    cmd->setHelp("Modify the active task...");
    cmd->setKeybind("ctrl m");
    cmd->setExec([this](const std::any&){
            if (_activeTask)
                this->showTaskSettings(_activeTask);
            else 
                this->error("No active task to edit.");
            });

    cmd = _cmds.createCommand<ml::GuiCommand>("Open Project", "open");
    cmd->setHelp("Open an existing project...");
    cmd->setKeybind("ctrl o");
    cmd->setExec([this](const std::any&){this->open();});

    cmd = _cmds.createCommand<ml::GuiCommand>("Save Project", "save");
    cmd->setHelp("Save the current project...");
    cmd->setKeybind("ctrl s");
    cmd->setExec([this](const std::any&){this->saveCurrentProject();});

    cmd = _cmds.createCommand<ml::GuiCommand>("Save Project as...", "save-as");
    cmd->setHelp("Save the current project as...");
    cmd->setKeybind("ctrl S");
    cmd->setExec([this](const std::any&){this->saveCurrentProjectAs();});

    auto getargs = [this]() -> json
    {
        json args;
        if(_taskSettingsWindow)
            args = _taskSettingsWindow->data();
        return args;
    };
    _createOrModifyTaskCmd = this->createBackendCommand(_backend, "create-or-modify-task", getargs, [this](const json& response){this->onTaskCreatedOrModified(response);}).get();
    _createOrModifyTaskCmd->setName("Create/Modify Task");
    _createOrModifyTaskCmd->setSync(true);

    auto nprj = this->createBackendCommand(_backend, "New Project", "new-project", "new-project", json::object(), [this](const json&){this->onNewProject();});
    nprj->setHelp("Create a new project...");
    nprj->setKeybind("ctrl n");

    auto rmargs = [this]() -> json
    {
        json args;    
        args["id"] = json::array();
        for (auto task : _selectedTasks)
            args["id"].push_back(task->data()["id"]);

        return args;
    };

    auto rmcb = [this](const json& response)
    {
        if (!response.contains("data"))
            return;

        auto tasks = this->tasks(response["data"]);
        this->removeTasks(tasks);
    };
    _removeTaskCmd = this->createBackendCommand(_backend, "remove-task", rmargs, rmcb).get();
    _removeTaskCmd->setName("Remove Task");
    _removeTaskCmd->setKeybind("d");

    auto getfilepath = [this]() -> json
    {
        json args;
        args["filepath"] = _currentFile;
        return args;
    };

    auto savecb = [this](const json& response)
    {
        storage::set("last-project", _currentFile);  
        this->addToRecentFiles(_currentFile);
    };

    _saveCmd = this->createBackendCommand(_backend, "Save", "save-gui", "save", getfilepath, savecb).get();

    auto opencb = [this](const json& response)
    {
        storage::set("last-project", _currentFile);  
        this->deserialize(response["data"]);
        this->addToRecentFiles(_currentFile);

        _fxpipeW->setTitle(_currentFile);
    };

    _openCmd = this->createBackendCommand(_backend, "Open", "open-gui", "open", getfilepath, opencb).get();


    auto openrecent = [this](const std::any& args)
    {
        this->showOpenRecentWindow();
    };
    cmd = this->cmds().createCommand<ml::GuiCommand>("Open Recent", "open-recent", openrecent);
    cmd->setHelp("Open a recent project...");
    cmd->setKeybind("ctrl O");

    auto reodersargs = [this]() -> json
    {
        json args;
        args["tasks"] = json::array();
        TaskView* view = fxpipeW()->taskView();
        if (this->activeTaskView())
            view = this->activeTaskView();
        if (view->taskParent())
            args["parent"] = view->taskParent()->id();
        for (auto& task : view->tasks())
            args["tasks"].push_back(task->id());

        return args;
    };
    _reorderTasksCmd = this->createBackendCommand(_backend, "Reorder Tasks", "reorder-tasks", "reorder-tasks", reodersargs).get();

    auto reparentedcb = [this](const json& res)
    {
        if (!res.contains("data"))
            return;
        if (!res["data"].contains("reparented"))
            return;

        std::string parent;
        if (res["data"].contains("parent"))
            parent = res["data"]["parent"];

        auto tasks = this->tasks(res["data"]["reparented"]);
        this->removeTasks(tasks);

        TaskView* view = _fxpipeW->taskView();
        if (!parent.empty())
        {
            auto parentTask_r = this->task(parent);
            if (parentTask_r.success)
                view = parentTask_r.value->tasksView();
        }
        for (const auto& taskdata : res["data"]["reparented"])
            view->createTask(taskdata);
    };
    _reparentTaskCmd = this->createBackendCommand(_backend, "Reparent Task", "reparent-task", "reparent").get();
    _reparentTaskCmd->addCallback(reparentedcb);

    auto opentaskargs = [this]() -> json
    {
        json data;
        if(!_activeTask)
            return data;
        data["id"] = _activeTask->data()["id"];
        data["event"] = "open";
        return data;
    };
    _openTaskCmd = this->createBackendCommand(_backend, "Open Task", "open-task", "emit-task-event", opentaskargs).get();

    cmd = this->cmds().createCommand<ml::GuiCommand>("Set Project Settings", "open-project-settings", [this](const std::any&){this->showProjectSettings();});
    cmd->setKeybind("alt s");

    cmd = this->cmds().createCommand<FxPipeCommand>("Show View Menu", "task-show-view-menu");
    cmd->setExec([this](const std::any&){_fxpipeW->showMenu("view");});
    cmd->setHelp("Show the view menu.");
    cmd->setKeybind("V");

    cmd = this->cmds().createCommand<ml::GuiCommand>("Refresh", "refresh-from-backend");
    cmd->setExec([this](const std::any&){this->updateAllFromBackend();});
    cmd->setHelp("Refresh all the data display in the GUI from the backend process.");
    cmd->setKeybind("ctrl r");

    auto newversionargs = [this]() -> json
    {
        if (!_newVersionWindow)
            return json::object();
        return _newVersionWindow->data();
    };

    auto newversioncb = [this](const json& response)
    {
        if (_newVersionWindow)
            _newVersionWindow->hide();
    };
    _newVersionCmd = this->createBackendCommand(_backend, "New Version", "new-version", "create-new-version", newversionargs, newversioncb).get();

    auto nversion = [this](const std::any&)
    {
        this->showNewVersionWindow();
    };
    cmd = this->cmds().createCommand<ml::GuiCommand>("New Version", "new-version-gui");
    cmd->setExec(nversion);
    cmd->setHelp("Create a new version of the project.");
    cmd->setKeybind("ctrl v");

    cmd = this->cmds().createCommand<ml::GuiCommand>("Search", "show-search");
    cmd->setExec([this](const std::any&){this->toggleSearch();});
    cmd->setHelp("Search for a specific task... (toggle)");
    cmd->setKeybind("slash");
    
    this->createSelectionCommands();
    this->createViewCommands();
    this->createTaskCommands();
    this->createTaskViewCommands();

    this->createSearchCommand();

    auto goalscmd = this->cmds().createCommand<ml::GuiCommand>("Manage Goals", "show-goals", [this](const std::any&){this->showGoals();});
}

void FxPipe::createSelectionCommands()
{
    auto cmd = this->cmds().createCommand<FxPipeCommand>("Select All", "select-all");	
    cmd->setExecForActiveTaskView([this](TaskView* activeView){this->selectAllTasksInView(activeView);});
    cmd->setHelp("Select all tasks in the active view.");
    cmd->setKeybind("ctrl a");

    cmd = this->cmds().createCommand<FxPipeCommand>("Deselect All", "deselect-all");	
    cmd->setExecGlobal([this]{this->replaceSelection(ml::Vec<Task*>());});
    cmd->setHelp("Deselect all tasks.");
    cmd->setKeybind("ctrl A");
}

void FxPipe::createViewCommands()
{
    this->cmds().createCommand<ml::GuiCommand>("Toggle Done", "view-toggle-from-state-done", 
            [this](const std::any& args){_fxpipeW->toggleTasks(std::any_cast<TaskStateType>(args));}, DONE);

    this->cmds().createCommand<ml::GuiCommand>("Toggle Started", "view-toggle-from-state-started", 
            [this](const std::any& args){_fxpipeW->toggleTasks(std::any_cast<TaskStateType>(args));}, STARTED);

    this->cmds().createCommand<ml::GuiCommand>("Toggle Archived", "view-toggle-from-state-archived", 
            [this](const std::any& args){_fxpipeW->toggleTasks(std::any_cast<TaskStateType>(args));}, ARCHIVED);

    this->cmds().createCommand<ml::GuiCommand>("Toggle Low", "view-toggle-from-state-low", 
            [this](const std::any& args){_fxpipeW->toggleTasks(std::any_cast<TaskStateType>(args));}, LOW);

    this->cmds().createCommand<ml::GuiCommand>("Toggle Medium", "view-toggle-from-state-medium", 
            [this](const std::any& args){_fxpipeW->toggleTasks(std::any_cast<TaskStateType>(args));}, MEDIUM);

    this->cmds().createCommand<ml::GuiCommand>("Toggle High", "view-toggle-from-state-high", 
            [this](const std::any& args){_fxpipeW->toggleTasks(std::any_cast<TaskStateType>(args));}, HIGH);

    this->cmds().createCommand<ml::GuiCommand>("Toggle Urgent", "view-toggle-from-state-urgent", 
            [this](const std::any& args){_fxpipeW->toggleTasks(std::any_cast<TaskStateType>(args));}, URGENT);

    this->cmds().createCommand<ml::GuiCommand>("Toggle This Week", "view-toggle-from-state-this-week", 
            [this](const std::any& args){_fxpipeW->toggleTasks(std::any_cast<TaskStateType>(args));}, THIS_WEEK);

    this->cmds().createCommand<ml::GuiCommand>("Toggle This Month", "view-toggle-from-state-this-month", 
            [this](const std::any& args){_fxpipeW->toggleTasks(std::any_cast<TaskStateType>(args));}, THIS_MONTH);

    this->cmds().createCommand<ml::GuiCommand>("Toggle Today", "view-toggle-from-state-today", 
            [this](const std::any& args){_fxpipeW->toggleTasks(std::any_cast<TaskStateType>(args));}, TODAY);

    this->cmds().createCommand<ml::GuiCommand>("Show All", "view-show-all", 
            [this](const std::any& args){_fxpipeW->showAllTasks();});
}

void FxPipe::createTaskCommands()
{
    auto cmd = this->cmds().createCommand<FxPipeCommand>("Toggle", "task-toggle-body");
    cmd->setExecForSelectedTasks([this](Task* task){task->toggleBody();});
    cmd->setHelp("Show/Hide the Task content and its subtasks.");
    cmd->setKeybind("h");

    cmd = this->cmds().createCommand<FxPipeCommand>("Mark as Done", "task-set-done");
    cmd->setExecForSelectedTasks([this](Task* task){task->setStatus(Task::DONE, true);});
    cmd->setHelp("Mark the task as done.");
    cmd->setKeybind("v");
    cmd->toQueue().push([this]{this->updateAllProgressesFromBackend();});

    cmd = this->cmds().createCommand<FxPipeCommand>("Mark as Archived", "task-toggle-archived");
    cmd->setExecForSelectedTasks([this](Task* task){
                task->toggleArchived();
            });
    cmd->setHelp("Toggle the task as archived.");

    cmd = this->cmds().createCommand<FxPipeCommand>("Mark as Started", "task-set-started");
    cmd->setExecForSelectedTasks([this](Task* task){task->setStatus(Task::STARTED, true);});
    cmd->setHelp("Mark the task as started.");
    cmd->setKeybind("s");
    cmd->toQueue().push([this]{this->updateAllProgressesFromBackend();});

    cmd = this->cmds().createCommand<FxPipeCommand>("Open Task", "task-open-gui-command");
    cmd->setExecForActiveTask([this](Task* task){_openTaskCmd->exec();});
    cmd->setHelp("Open the task.");
    cmd->setKeybind("o");

    cmd = this->cmds().createCommand<FxPipeCommand>("Reparent", "task-reparent");

    auto cmdexec = [this](const ml::Vec<Task*>& tasks)
    {
        if (!this->activeTask())
            return;
        json args;
        args["parent"] = this->activeTask()->id();
        args["id"] = json::array();
        for (auto task : tasks)
        {
            if (task->id() == this->activeTask()->id())
                continue;
            args["id"].push_back(task->id());
        }

        _reparentTaskCmd->execJson(args);
    };
    cmd->setExecForSelectedTasks(cmdexec);
    cmd->setHelp("Reparent the selected Tasks in the active one.");
    cmd->setKeybind("ctrl p");

    auto cmdexec2 = [this](const ml::Vec<Task*>& tasks)
    {
        json args;
        args["id"] = json::array();
        for (auto task : tasks)
            args["id"].push_back(task->id());

        _reparentTaskCmd->execJson(args);
    };
    cmd = this->cmds().createCommand<FxPipeCommand>("Unparent", "task-unparent");
    cmd->setExecForSelectedTasks(cmdexec2);
    cmd->setHelp("Unparent the Task : put it in the uppest View.");
    cmd->setKeybind("alt p");

    auto cmdexec3 = [this](const ml::Vec<Task*>& tasks)
    {
        json args;
        auto tret = this->activeTask()->parent();
        if (tret.success)
        {
            auto pret = tret.value->parent();
            if (pret.success)
                args["parent"] = pret.value->id();
        }
        args["id"] = json::array();
        for (auto task : tasks)
        {
            if (task->id() == args["parent"])
                continue;
            args["id"].push_back(task->id());
        }

        _reparentTaskCmd->execJson(args);
    };
    cmd = this->cmds().createCommand<FxPipeCommand>("Reparent up", "task-reparent-up");
    cmd->setExecForSelectedTasks(cmdexec3);
    cmd->setHelp("Reparent the Task up : Put in the View above it.");
    cmd->setKeybind("P");

    cmd = this->cmds().createCommand<FxPipeCommand>("Duplicate", "task-duplicate");
    cmd->setExecForSelectedTasks([this](Task* task){task->duplicate();});
    cmd->setHelp("Duplicate the selected tasks");
    cmd->setKeybind("ctrl d");

    cmd = this->cmds().createCommand<FxPipeCommand>("Move up", "task-move-up");
    cmd->setExecForActiveTaskView([this](TaskView* view){view->moveSelectedTasks(-1);});
    cmd->setHelp("Move the tasks up.");
    cmd->setKeybind("ctrl Up");
    cmd = this->cmds().createCommand<FxPipeCommand>("Move down", "task-move-down");
    cmd->setExecForActiveTaskView([this](TaskView* view){view->moveSelectedTasks(1);});
    cmd->setHelp("Move the tasks down.");
    cmd->setKeybind("ctrl Down");
}

void FxPipe::createTaskViewCommands()
{
}

void FxPipe::deserialize(const json& data)
{
    std::string oldActive;
    ml::Vec<std::string> oldSelection;
    ml::Vec<std::string> shown;

    if (_activeTask)
        oldActive = _activeTask->id();
    if (_selectedTasks.size())
    {
        for (auto task : _selectedTasks)
            oldSelection.push_back(task->id());
    }

    auto tasks = this->allTasks();
    for (auto task : tasks)
    {
        if (task->bodyShown())
            shown.push_back(task->id());
    }

    lg(shown.size() << " tasks are shown before deserialization...");

    _activeTask = nullptr;
    _selectedTasks.clear();

    _fxpipeW->taskView()->clear();
    if (_projectSettingsWindow)
        _projectSettingsWindow->reset();

    if (data.contains("tasks"))
    {
        for (const auto& dt : data["tasks"])
            _fxpipeW->taskView()->createTask(dt);
    }

    if (data.contains("project-settings"))
    {
        _projectSettings = data["project-settings"];
        _fxpipeW->updateProjectSettings();
    }

    if (!oldActive.empty())
        this->setActiveTask(oldActive);
    if (oldSelection.size())
        this->replaceSelection(oldSelection);

    auto shownTasks = this->tasks(shown);
    lg("found back " << shownTasks.size() << " tasks to reshow.");
    for (auto task : shownTasks)
        task->showBody();
}

void FxPipe::onTaskCreatedOrModified(const json& response)
{
    const json& resTask = response["data"];

    if (!resTask.contains("id"))
    {
        this->error("Error creating task, no id returned.");
        return;
    }

    auto task_re = this->task(resTask["id"].get<std::string>());
    if (!task_re.success)
    {
        if (resTask.contains("parent") && resTask["parent"].get<std::string>() != "")
        {
            auto rtask = this->task(resTask["parent"].get<std::string>());
            if (rtask.success)
                rtask.value->tasksView()->createTask(resTask);
        }
        else 
            _fxpipeW->taskView()->createTask(resTask);
    }
    else 
        task_re.value->update(resTask);

    if (_taskSettingsWindow && _taskSettingsWindow->visible())
        _taskSettingsWindow->hide();
}

void FxPipe::saveCurrentProject()
{
    if (_currentFile.empty())	
    {
        this->saveCurrentProjectAs();
        return;
    }
    _saveCmd->exec();
}

void FxPipe::saveCurrentProjectAs()
{
    auto onselected = [this](const std::string& filepath)
    {
        _currentFile = filepath;
        this->saveCurrentProject();
    };
    this->saveFile("Save project...", "", onselected);	
}

void FxPipe::open(const std::string& path)
{
    _currentFile = path;	
    if (_currentFile.empty())
    {
        this->openFile("Open project...", "", [this](const std::string& filepath){this->open(filepath);});
        return;
    }

    _openCmd->exec();
}

void FxPipe::onNewProject()
{
    _currentFile = "";
    this->deserialize(json::object());
    _projectSettings = json::object();
    if (_projectSettingsWindow)
        _projectSettingsWindow->reset();
}

void FxPipe::openLastProject()
{
    _currentFile = storage::get<std::string>("last-project");	
    if (_currentFile.empty())
        return;
    _openCmd->exec();
}

void FxPipe::startBackendProcess()
{
    lg("(gui) FxPipe::startBackendProcess");
    _backendPath = _paths.fxpipe_core + files::sep() + "fxpipe-core";
    _backend = process::start(_backendPath);
}

void FxPipe::createWindows()
{
    this->createMainWindow<MainWindow>();
    _fxpipeW = (MainWindow*)_main;
}

FxPipe::~FxPipe()
{
    _backend->terminate();
    fxpipe::_fxpipe = nullptr;	
}

void FxPipe::setActiveTask(Task* task)
{
    if (_activeTask == task)	
        return;

    _activeTask = task;
    _events.emit("active-task-changed");
}

void FxPipe::setActiveTask(const std::string& id)
{
    auto task_r = this->task(id);	
    if (task_r.success)
        this->setActiveTask(task_r.value);
}

void FxPipe::showTaskSettings(Task* task)
{
    if (!_taskSettingsWindow)
        _taskSettingsWindow = this->createWindow<TaskSettingsWindow>(_fxpipeW).get();

    _taskSettingsWindow->show();	
    if (task)
        _taskSettingsWindow->setFromTask(task);
}

void FxPipe::showTaskSettingsForCreation()
{
    if (!_taskSettingsWindow)
        _taskSettingsWindow = this->createWindow<TaskSettingsWindow>(_fxpipeW).get();
    else 
        _taskSettingsWindow->reset();

    _taskSettingsWindow->show();	
    if (_activeTask)
        _taskSettingsWindow->setParent(_activeTask->data()["id"], _activeTask->data()["name"]);
}

ml::Ret<Task*> FxPipe::task(const std::string& id)
{
    for (auto& task : _fxpipeW->taskView()->tasks())	
    {
        if (task->id() == id)
            return ml::ret::success(task.get());
        else 
        {
            auto res = task->child(id);
            if (res.success)
                return res;
        }
    }

    return ml::ret::fail<Task*>("Task not found");
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

ml::Vec<Task*> FxPipe::tasks(const json& ids)
{
    ml::Vec<std::string> ids_arr;	
    for (const auto& id : ids)
    {
        if (id.is_object())
        {
            try
            {
                ids_arr.push_back(id["id"].get<std::string>());
            }
            catch(const std::exception& e)
            {
                continue;
            }
        }
        else if (id.is_string())
            ids_arr.push_back(id.get<std::string>());
    }
    return this->tasks(ids_arr);
}

ml::Vec<Task*> FxPipe::allTasks()
{
    ml::Vec<Task*> tasks;
    for (auto& task : _fxpipeW->taskView()->tasks())	
    {
        tasks.push(task.get());
        for (const auto c : task->children(true))
            tasks.push(c);
    }
    return tasks;
}

void FxPipe::showProjectSettings()
{
    if (!_projectSettingsWindow)	
        _projectSettingsWindow = this->createWindow<ProjectSettingsWindow>(_fxpipeW).get();
    _projectSettingsWindow->show();
    _projectSettingsWindow->setFromData(_projectSettings);
}

void FxPipe::clearSelection(bool emitEvent)
{
    _selectedTasks.clear();	
    if (emitEvent)
        _events.emit("selection-changed");
}

void FxPipe::addToSelection(Task* task, bool emitEvent)
{
    if(task->selected())
        return;
    _selectedTasks.push(task);	
    if (emitEvent)
        _events.emit("selection-changed");
}

void FxPipe::removeFromSelection(Task* task, bool emitEvent)
{
    if (!task->selected())
        return;
    _selectedTasks.remove(task);	
    if (emitEvent)
        _events.emit("selection-changed");
}

void FxPipe::replaceSelection(Task* task,bool emitEvent)
{
    this->clearSelection(false);
    this->addToSelection(task, emitEvent);
}

void FxPipe::addToSelection(const ml::Vec<Task*>& tasks,bool emitEvent)
{
    for (auto task : tasks)
        this->addToSelection(task, false);
    if (emitEvent)
        _events.emit("selection-changed");	
}

void FxPipe::removeFromSelection(const ml::Vec<Task*>& tasks,bool emitEvent)
{
    for (auto task : tasks)
        this->removeFromSelection(task, false);
    if (emitEvent)
        _events.emit("selection-changed");
}

void FxPipe::replaceSelection(const ml::Vec<Task*>& tasks,bool emitEvent)
{
    this->clearSelection(false);
    this->addToSelection(tasks, emitEvent);
}

void FxPipe::replaceSelection(const ml::Vec<std::string>& tasks, bool emitEvent)
{
    auto tasks_ptrs = this->tasks(tasks);	
    this->replaceSelection(tasks_ptrs, emitEvent);
}

void FxPipe::addToSelection(const ml::Vec<std::unique_ptr<Task>>& tasks, bool emitEvent)
{
    for (auto& task : tasks)
        this->addToSelection(task.get(), false);
    if (emitEvent)
        _events.emit("selection-changed");	
}

void FxPipe::removeFromSelection(const ml::Vec<std::unique_ptr<Task>>& tasks, bool emitEvent)
{
    for (auto& task : tasks)
        this->removeFromSelection(task.get(), false);
    if (emitEvent)
        _events.emit("selection-changed");
}

void FxPipe::replaceSelection(const ml::Vec<std::unique_ptr<Task>>& tasks, bool emitEvent)
{
    this->clearSelection(false);
    this->addToSelection(tasks, emitEvent);
}

void FxPipe::selectAllTasksInView(TaskView* view,bool emitEvent)
{
    this->replaceSelection(this->activeTaskView()->tasks(), emitEvent);
}

void FxPipe::_onSelectionChanged()
{
    for (auto task : this->allTasks())
        task->removeCssClass("selected");
    for (auto task : _selectedTasks)
        task->addCssClass("selected");

    if (_selectedTasks.size() > 0)
        this->setActiveTask(_selectedTasks.last());
    else 
        this->setActiveTask(nullptr);
}

void FxPipe::_onActiveTaskChanged()
{
    for (auto task : this->allTasks())
        task->removeCssClass("active");

    if (_activeTask)
        _activeTask->addCssClass("active");
}

TaskView* FxPipe::activeTaskView() const
{
    if(_activeTaskView)
        return _activeTaskView;
    else
        return _fxpipeW->taskView();
}

void FxPipe::removeTasks(const ml::Vec<Task*>& tasks)
{
    lg("Removing " << tasks.size() << " tasks");
    for (auto t : tasks)	
        t->remove();
    return;
}

void FxPipe::updateAllFromBackend()
{
    auto cb = [this](const json& response)
    {
        if (!response.contains("data"))
            return;
        this->deserialize(response["data"]);
    };
    this->executeBackendCommand(_backend, "get-all-data", json::object(), cb);
}

void FxPipe::updateAllProgressesFromBackend()
{
    auto cb = [this](const json& response)
    {
        if (!response.contains("data"))
            return;
        this->updateAllProgresses(response["data"]);
    };
    this->executeBackendCommand(_backend, "get-all-data", json::object(), cb);
}


void updateTaskProgressRecc(Task* task, const json& data)
{
    if (data.contains("progress"))
        task->setProgress(data["progress"]);
    if (!data.contains("subtasks"))
        return;
    for (const auto& subtask : data["subtasks"])
    {
        auto task = fxpipe::get()->task(subtask["id"]);
        if (!task.success)
            continue;
        updateTaskProgressRecc(task.value, subtask);
    }
}

void FxPipe::updateAllProgresses(const json& data)
{
    if (!data.contains("tasks"))
        return;
    for (const auto& task : data["tasks"])
    {
        auto tret = this->task(task["id"]);
        if (!tret.success)
            continue;
        updateTaskProgressRecc(tret.value, task);
    }
}

void FxPipe::showNewVersionWindow()
{
    if (!_newVersionWindow)
        _newVersionWindow = this->createWindow<NewVersionWindow>(_fxpipeW).get();
    _newVersionWindow->reset();	
    _newVersionWindow->show();	
}

void FxPipe::showOpenRecentWindow()
{
    if (!_openRecentWindow)
        _openRecentWindow = this->createWindow<OpenRecentWindow>(_fxpipeW).get();
    _openRecentWindow->show();	
}

void FxPipe::addToRecentFiles(const std::string& path)
{
    auto recent = storage::get<json>("recent-files");	
    bool contains = false;
    for (const auto& f : recent)
    {
        if (f.contains("filepath") && f["filepath"] == path)
        {
            contains = true;
            break;
        }
    }

    auto onres = [this, recent, contains, path](const json& res) mutable
    {
        if (!res.contains("data"))
            return;
        if (contains)
        {
            for (auto& f : recent)
            {
                if (f.contains("filepath") && f["filepath"] == path)
                {
                    f["name"] = res["data"]["name"];
                    break;
                }
            }
        }
        else 
        {
            json rfile = json::object();
            rfile["filepath"] = path;
            rfile["name"] = res["data"]["name"];
            recent.push_back(rfile);
        }
        storage::set("recent-files", recent);
    };

    this->executeBackendCommand(_backend, "project-settings", json::object(), onres);
}

void FxPipe::toggleSearch()
{
    if (_fxpipeW->search().visible())
        _fxpipeW->search().hide();	
    else 
    {
        _fxpipeW->search().show();    
        _fxpipeW->search().focus();
    }
}

void FxPipe::sendSearch(const std::string& text)
{
    _searchCmd->execJson({{"query", text}});
}

void FxPipe::createSearchCommand()
{
    auto cb = [this](const json& res)
    {
        if (!res.contains("data")) 
            return;
        if (!res["data"].is_array())
            return;

        if (res["data"].size() == 0)
        {
            for (auto& task : this->allTasks())
            {
                if (!task->archived())
                    task->show();
                else 
                    task->hide();
            }

            for (auto& task : this->allTasks())
                task->hideBody();
            return;
        }

        for (auto& task : this->allTasks())
        {
            task->hideBody();
            task->hide();
        }

        for (auto& task : res["data"])
        {
            auto tret = this->task(task["id"]);
            if (tret.success)
                tret.value->show();
        }
    };
    _searchCmd = this->createBackendCommand(_backend, "Search", "send-search", "search", json::object(), cb).get();
    _searchCmd->setSync(true);
}

void FxPipe::initGoals()
{
    _goalsWindow = this->createWindow<GoalsWindow>(_fxpipeW).get();	
    this->queue([this]{
        _goalsWindow->hide(); 
    });
    auto f = [this]{
        _goalsWindow->checkForPeriodPassed();
    };

    this->setTimeout(f, 5000);
}

void FxPipe::showGoals()
{
    _goalsWindow->show();
}

namespace fxpipe
{
    FxPipe* get(){return _fxpipe;}
}

