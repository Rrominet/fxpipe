#pragma once
#include <memory>
#include "mlgui.2/src/Window.h"
#include "mlgui.2/src/Property.h"

namespace ml
{
    class Box;
}

struct SessionData
{
    std::string name;
    unsigned int duration = 8*60*60; //in sec
    unsigned int duration_done = 0; //in sec
    bool done = false;
};

struct SessionGui
{
    SessionGui() :  
        name("Session Name", "", "The name (or type) of the work session"),
        duration("Duration", 8, 0, 1000, "The duration of the session in hours"),
        duration_done("Already done", 8, 0, 1000, "The duration that has already been done in hours")
    {}

    ml::StringProperty name;
    ml::FloatProperty duration; //in hours
    ml::FloatProperty duration_done; //same, in hours
};


struct SessionDrawn
{
    private :
        std::shared_ptr<bool> _alive;

    public : 
    SessionDrawn(ml::Box* parent) : 
        parent(parent),
        done("Done", false) {
            _alive = std::make_shared<bool>(true);
        }

    ~SessionDrawn() {
        *_alive = false;
    }

    ml::Box* parent;
    ml::Box* box = nullptr;
    ml::Label* label = nullptr;
    ml::Label* duration = nullptr;
    ml::BoolProperty done;
    SessionData* data = nullptr;
    std::weak_ptr<bool> alive() const {return _alive;}
};

class TimeTrackingWin : public ml::Window
{
    public:
        TimeTrackingWin(ml::App* app) : ml::Window(app){};
        TimeTrackingWin(ml::App* app,ml::Window* parent) : ml::Window(app, parent){};
        virtual ~TimeTrackingWin();

        virtual void init() override;

        void save();
        void load();
        json serializeSessions() const;
        void deserializeSessions(const json& data);

        void showNewSessionGui();
        void createCommands();
        void createMenus();

        void createNewSession();
        void modifySession();
        void showModifySessionGui();
        void showMainUI();

        void redrawSessions();
        void createCtx();

        void removeSessionFromData(SessionData* sessionData);
        void setActiveSession(SessionDrawn* sessionDrawn);
        
        void startTrack(SessionData* sessionData);
        void stopTrack(bool showError = true);

    protected : 
        ml::Box* _sessionsBox;
        ml::Box* _newSessionBox;
        SessionGui _sessionGui;
        ml::Vec<std::unique_ptr<SessionData>> _sessions;
        ml::Vec<std::unique_ptr<SessionDrawn>> _sessionsDrawn;
        void _drawSession(SessionData* sessionData);
        SessionDrawn* _fromIndex(int index);
        int _indexFromSessionDrawn(SessionDrawn* sessionDrawn);

        int _activeSession = -1;

        int _beingTracked = -1;
        SessionData* _modifiedSession = nullptr;
        SessionData* _dataFromIndex(int index);
        int _indexFromData(SessionData* sessionData);

        int _trackIntervalId = -1;
        
        ml::Button *_createBtn = nullptr;
        ml::Button *_modifyBtn = nullptr;
};
