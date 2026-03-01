#pragma once
#include "mlgui.2/src/Window.h"
#include "mlprocess.h"
#include "./enums.h"

namespace ml
{
    class Box;
    class Tabs;
}

class TaskView;
class MainWindow : public ml::Window
{
    public:
        MainWindow(ml::App* app);
        MainWindow(ml::App* app, ml::Window* parent);
        virtual ~MainWindow();

        virtual void init() override;
        void createMenus();

        TaskView* taskView() { return _taskView.get(); }
        void updateProjectSettings();
        void toggleTasks(TaskStateType type);
        void showAllTasks();

    protected : 
        void _setEvents();

        std::unique_ptr<TaskView> _taskView;

        bool _done_visible = true; //DONE
        bool _started_visible = true; //STARTED
        bool _archived_visible = true; //ARCHIVED
        bool _low_visible = true; //LOW
        bool _medium_visible = true; //MEDIUM
        bool _high_visible = true; //HIGH
        bool _urgent_visible = true; //URGENT
        bool _this_week_visible = true; //THIS_WEEK
        bool _this_month_visible = true; //THIS_MONTH
        bool _today_visible = true; //TODAY
};
