#pragma once
#include "mlgui.2/src/App.h"
#include "mlprocess.h"
#include "./Paths.h"
#include "Ret.h"
#include "Events.h"

namespace ml
{
    class GuiBackendCommand;
}

class MainWindow;
class Task;
class TaskView;
class TaskSettingsWindow;
class ProjectSettingsWindow;
class NewVersionWindow;
class OpenRecentWindow;
class FxPipe : public ml::App
{
    public:
        FxPipe(int argc, char *argv[]);
        virtual ~FxPipe();
        void startBackendProcess();
        void createCommands();
        void createSelectionCommands();
        void createViewCommands();
        void createTaskCommands();
        void createTaskViewCommands();
        void createWindows();
        void setEvents();

        void showTaskSettings(Task* task=nullptr);
        void showTaskSettingsForCreation();
        void onTaskCreatedOrModified(const json& response);

        void showProjectSettings();

        void saveCurrentProject();
        void saveCurrentProjectAs();
        void open(const std::string& path="");
        void openLastProject();
        void onNewProject();

        void deserialize(const json& data);
        void updateAllProgresses(const json& data);
        
        ml::Ret<Task*> task(const std::string& id);

        //return all the tasks reccursively
        ml::Vec<Task*> allTasks();
        ml::Vec<Task*> tasks(const ml::Vec<std::string>& ids);

        //json should be a json::array()
        ml::Vec<Task*> tasks(const json& ids);
        void clearSelection(bool emitEvent=true);

        void addToSelection(Task* task, bool emitEvent=true);
        void removeFromSelection(Task* task, bool emitEvent=true);
        void replaceSelection(Task* task, bool emitEvent=true);

        void addToSelection(const ml::Vec<Task*>& tasks, bool emitEvent=true);
        void removeFromSelection(const ml::Vec<Task*>& tasks, bool emitEvent=true);
        void replaceSelection(const ml::Vec<Task*>& tasks, bool emitEvent=true);

        void addToSelection(const ml::Vec<std::unique_ptr<Task>>& tasks, bool emitEvent=true);
        void removeFromSelection(const ml::Vec<std::unique_ptr<Task>>& tasks, bool emitEvent=true);
        void replaceSelection(const ml::Vec<std::unique_ptr<Task>>& tasks, bool emitEvent=true);
        void replaceSelection(const ml::Vec<std::string>& tasks, bool emitEvent=true);

        void selectAllTasksInView(TaskView* view, bool emitEvent=true);

        void setActiveTask(Task* task);
        void setActiveTask(const std::string& id);
        TaskView* activeTaskView() const;

        void removeTasks(const ml::Vec<Task*>& tasks);
        void updateAllFromBackend();
        void updateAllProgressesFromBackend();

        void showNewVersionWindow();
        void showOpenRecentWindow();

        void addToRecentFiles(const std::string& path);

    private : 
        MainWindow* _fxpipeW=nullptr; //bp cg

        std::string _backendPath;
        Process* _backend=nullptr; //bp cg
        Paths _paths; //bp cg

        //Last Task selected.
        Task* _activeTask=nullptr; //bp cg
        TaskView* _activeTaskView=nullptr; //bp s
        ml::Vec<Task*> _selectedTasks; //bp cg                           
        TaskSettingsWindow* _taskSettingsWindow=nullptr; //bp cg
        ProjectSettingsWindow* _projectSettingsWindow=nullptr; //bp cg
        NewVersionWindow* _newVersionWindow=nullptr; //bp cg
        OpenRecentWindow* _openRecentWindow=nullptr; //bp cg
        TaskView* _draggedTaskView = nullptr; //bp cgs

        ml::GuiBackendCommand* _createOrModifyTaskCmd = nullptr; //bp cg
        ml::GuiBackendCommand* _openCmd = nullptr; //bp cg
        ml::GuiBackendCommand* _saveCmd = nullptr; //bp cg
        ml::GuiBackendCommand* _removeTaskCmd = nullptr; //bp cg
        ml::GuiBackendCommand* _reorderTasksCmd = nullptr; //bp cg
        ml::GuiBackendCommand* _openTaskCmd = nullptr; //bp cg
        ml::GuiBackendCommand* _reparentTaskCmd = nullptr; //bp cg
        ml::GuiBackendCommand* _newVersionCmd = nullptr; //bp cg

        std::string _currentFile = ""; //bp cgs

        json _projectSettings; //bp cgs
        ml::Events _events; //bp cg

        void _onSelectionChanged();
        void _onActiveTaskChanged();

    public : 
#include "./FxPipe_gen.h"
};

namespace fxpipe
{
    FxPipe* get();
}
