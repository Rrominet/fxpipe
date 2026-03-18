#include "./MainWindow.h"
#include "./FxPipe.h"
#include "./TaskView.h"

#include "mlgui.2/src/Box.h"
#include "mlgui.2/src/MenuBar.h"
#include "mlgui.2/src/Box.hpp"
#include "mlgui.2/src/Scrollable.hpp"

#include "./Task.h"

#define CONSTRUCT_PROPS _search("Search", "", "Search for a specific task...")

MainWindow::MainWindow(ml::App* app) : ml::Window(app), CONSTRUCT_PROPS {}
MainWindow::MainWindow(ml::App* app, ml::Window* parent) : ml::Window(app, parent), CONSTRUCT_PROPS {}
MainWindow::~MainWindow(){}

void MainWindow::init()
{
    ml::Window::init();	
    this->setSize(800, 540);
    this->setTitle("fxpipe");
    this->createMenus();
    _main->appendProp(&_search);
    _search.hide();

    _taskView = std::make_unique<TaskView>(&this->main()->content());

    _setEvents();
}

void MainWindow::_setEvents()
{
    _search.addOnUpdate([this]{
            fxpipe::get()->sendSearch(_search.value());
            });
    _search.addOnValid([this]{
                _search.hide();
            });
}

void MainWindow::createMenus()
{
    auto file = ml::app()->menus().create("file", "File");
    file->addCommand("new-project");
    file->addCommand("open-project-settings");
    file->addSeparator();
    file->addCommand("open");
    file->addCommand("open-recent");
    file->addSeparator();
    file->addCommand("save");
    file->addCommand("save-as");
    file->addSeparator();
    file->addCommand("new-version-gui");
    file->addSeparator();
    file->addCommand("quit");

    auto edit = ml::app()->menus().create("edit", "Edit");
    edit->addCommand("create-task");
    edit->addCommand("modify-task");
    edit->addCommand("open-task");
    edit->addSeparator();
    edit->addCommand("select-all");
    edit->addCommand("deselect-all");
    edit->addSeparator();
    edit->addCommand("remove-task");
    edit->addSeparator();
    edit->addCommand("task-move-up");
    edit->addCommand("task-move-down");
    edit->addSeparator();
    edit->addCommand("task-reparent");
    edit->addCommand("task-unparent");
    edit->addCommand("task-reparent-up");
    edit->addSeparator();
    edit->addCommand("task-toggle-archived");
    edit->addCommand("task-set-done");
    edit->addCommand("task-set-started");

    auto view = ml::app()->menus().create("view", "View");
    view->addCommand("show-search");
    view->addSeparator();
    view->addCommand("view-toggle-from-state-done");
    view->addCommand("view-toggle-from-state-started");
    view->addCommand("view-toggle-from-state-archived");
    view->addSeparator();
    view->addCommand("view-toggle-from-state-low");
    view->addCommand("view-toggle-from-state-medium");
    view->addCommand("view-toggle-from-state-high");
    view->addCommand("view-toggle-from-state-urgent");
    view->addSeparator();
    view->addCommand("view-toggle-from-state-this-week");
    view->addCommand("view-toggle-from-state-this-month");
    view->addCommand("view-toggle-from-state-today");
    view->addSeparator();
    view->addCommand("view-show-all");
    view->addSeparator();
    view->addCommand("refresh-from-backend");

    auto tools = ml::app()->menus().create("tools", "Tools");
    tools->addCommand("show-goals");

    auto help = ml::app()->menus().create("help", "Help");
    help->addCommand("about");

    _menuBar->addMenu("file");
    _menuBar->addMenu("edit");
    _menuBar->addMenu("view");
    _menuBar->addMenu("tools");
    _menuBar->addMenu("help");

    auto tskctx = ml::app()->menus().create("task-context", "Task Menu");
    tskctx->addCommand("create-task");
    tskctx->addSeparator();
    tskctx->addCommand("task-set-done");
    tskctx->addCommand("task-set-started");
    tskctx->addSeparator();
    tskctx->addCommand("task-open-gui-command");
    tskctx->addSeparator();
    tskctx->addCommand("remove-task");
    tskctx->addSeparator();
    tskctx->addCommand("task-unparent");
    tskctx->addCommand("task-reparent-up");
}

void MainWindow::updateProjectSettings()
{
    auto& data = fxpipe::get()->projectSettings();
    if(data.contains("name"))
        this->setTitle(data["name"]);
}

void MainWindow::toggleTasks(TaskStateType type)
{
    switch (type)
    {
        case DONE: 
            _done_visible = !_done_visible;
            break;
        case STARTED:
            _started_visible = !_started_visible;
            break;
        case ARCHIVED:
            _archived_visible = !_archived_visible;
            break;
        case LOW:
            _low_visible = !_low_visible;
            break;
        case MEDIUM:
            _medium_visible = !_medium_visible;
            break;
        case HIGH:
            _high_visible = !_high_visible;
            break;
        case URGENT:
            _urgent_visible = !_urgent_visible;
            break;
        case THIS_WEEK:
            _this_week_visible = !_this_week_visible;
            break;
        case THIS_MONTH:
            _this_month_visible = !_this_month_visible;
            break;
        case TODAY:
            _today_visible = !_today_visible;
            break;
    }
    
    for (auto t : fxpipe::get()->allTasks())	
    {
        switch (type)
        {
            case DONE: 
                if (t->status() == Task::DONE)
                    t->setVisible(_done_visible);
                break;
            case STARTED:
                if (t->status() == Task::STARTED)
                    t->setVisible(_started_visible);
                break;
            case ARCHIVED:
                if (t->data()["archived"].get<bool>())
                    t->setVisible(_archived_visible);
                break;
            case LOW:
                if (t->priority() == Task::LOW)
                    t->setVisible(_low_visible);
                break;
            case MEDIUM:
                if (t->priority() == Task::MEDIUM)
                    t->setVisible(_medium_visible);
                break;
            case HIGH:
                if (t->priority() == Task::HIGH)
                    t->setVisible(_high_visible);
                break;
            case URGENT:
                if (t->priority() == Task::URGENT)
                    t->setVisible(_urgent_visible);
                break;
            case THIS_WEEK:
                if (t->priority() == Task::THIS_WEEK)
                    t->setVisible(_this_week_visible);
                break;
            case THIS_MONTH:
                if (t->priority() == Task::THIS_MONTH)
                    t->setVisible(_this_month_visible);
                break;
            case TODAY:
                if (t->priority() == Task::TODAY)
                    t->setVisible(_today_visible);
                break;
        }
    }
}

void MainWindow::showAllTasks()
{
    _done_visible = true;
    _started_visible = true;
    _archived_visible = true;
    _low_visible = true;
    _medium_visible = true;
    _high_visible = true;
    _urgent_visible = true;
    _this_week_visible = true;
    _this_month_visible = true;
    _today_visible = true;
    
    for (auto t : fxpipe::get()->allTasks())
        t->setVisible(true);
}
