//This is a generated file, don't change it manually, it will be override when rebuild.

MainWindow* fxpipeW(){return _fxpipeW;}
const MainWindow* fxpipeW() const {return _fxpipeW;}


Process* backend(){return _backend;}
const Process* backend() const {return _backend;}


Paths& paths(){return _paths;}
const Paths& paths() const {return _paths;}


Task* activeTask(){return _activeTask;}
const Task* activeTask() const {return _activeTask;}


void setActiveTaskView(TaskView* activeTaskView){_activeTaskView = activeTaskView;}

ml::Vec<Task*>& selectedTasks(){return _selectedTasks;}
const ml::Vec<Task*>& selectedTasks() const {return _selectedTasks;}


TaskSettingsWindow* taskSettingsWindow(){return _taskSettingsWindow;}
const TaskSettingsWindow* taskSettingsWindow() const {return _taskSettingsWindow;}


ProjectSettingsWindow* projectSettingsWindow(){return _projectSettingsWindow;}
const ProjectSettingsWindow* projectSettingsWindow() const {return _projectSettingsWindow;}


NewVersionWindow* newVersionWindow(){return _newVersionWindow;}
const NewVersionWindow* newVersionWindow() const {return _newVersionWindow;}


OpenRecentWindow* openRecentWindow(){return _openRecentWindow;}
const OpenRecentWindow* openRecentWindow() const {return _openRecentWindow;}


TaskView* draggedTaskView(){return _draggedTaskView;}
const TaskView* draggedTaskView() const {return _draggedTaskView;}
void setDraggedTaskView(TaskView* draggedTaskView){_draggedTaskView = draggedTaskView;}

ml::GuiBackendCommand* createOrModifyTaskCmd(){return _createOrModifyTaskCmd;}
const ml::GuiBackendCommand* createOrModifyTaskCmd() const {return _createOrModifyTaskCmd;}


ml::GuiBackendCommand* openCmd(){return _openCmd;}
const ml::GuiBackendCommand* openCmd() const {return _openCmd;}


ml::GuiBackendCommand* saveCmd(){return _saveCmd;}
const ml::GuiBackendCommand* saveCmd() const {return _saveCmd;}


ml::GuiBackendCommand* removeTaskCmd(){return _removeTaskCmd;}
const ml::GuiBackendCommand* removeTaskCmd() const {return _removeTaskCmd;}


ml::GuiBackendCommand* reorderTasksCmd(){return _reorderTasksCmd;}
const ml::GuiBackendCommand* reorderTasksCmd() const {return _reorderTasksCmd;}


ml::GuiBackendCommand* openTaskCmd(){return _openTaskCmd;}
const ml::GuiBackendCommand* openTaskCmd() const {return _openTaskCmd;}


ml::GuiBackendCommand* reparentTaskCmd(){return _reparentTaskCmd;}
const ml::GuiBackendCommand* reparentTaskCmd() const {return _reparentTaskCmd;}


ml::GuiBackendCommand* newVersionCmd(){return _newVersionCmd;}
const ml::GuiBackendCommand* newVersionCmd() const {return _newVersionCmd;}


std::string currentFile(){return _currentFile;}
const std::string& currentFile() const {return _currentFile;}
void setCurrentFile(const std::string& currentFile){_currentFile = currentFile;}

json& projectSettings(){return _projectSettings;}
const json& projectSettings() const {return _projectSettings;}
void setProjectSettings(const json& projectSettings){_projectSettings = projectSettings;}

ml::Events& events(){return _events;}
const ml::Events& events() const {return _events;}


