#include "./TimeTrackingWin.h"
#include "mlgui.2/src/Box.h"
#include "mlgui.2/src/MessageDialog.h"
#include "storage.h"

#include "mlgui.2/src/Box.hpp"
#include "mlgui.2/src/Scrollable.hpp"
#include "mlgui.2/src/App.hpp"


TimeTrackingWin::~TimeTrackingWin()
{
    this->stopTrack(false);	
}

void TimeTrackingWin::init()
{
    ml::Window::init();	
    this->setTitle("Time Session Tracking");
    this->setSize(850, 400);
    this->createCommands();
    this->createMenus();

    _sessionsBox = this->main()->createBox().get();
    _sessionsBox->addCssClass("sessions");

    _newSessionBox = this->main()->createBox().get();
    _newSessionBox->addCssClass("new-session");
    _newSessionBox->appendProp(&_sessionGui.name);
    _newSessionBox->appendProp(&_sessionGui.duration);
    _newSessionBox->appendProp(&_sessionGui.duration_done);
    _createBtn = _newSessionBox->appendCommand(_cmds.command("create-new-session").get()).get();
    _modifyBtn = _newSessionBox->appendCommand(_cmds.command("modify-session").get()).get();

    _newSessionBox->hide();

    this->createCtx();
    this->load();
}

void TimeTrackingWin::save()
{
    storage::set("time-tracking-sessions", this->serializeSessions());
    this->setInfos("Saved.", 5000);
}

void TimeTrackingWin::showNewSessionGui()
{
    _newSessionBox->show();	
    _sessionsBox->hide();

    _createBtn->show();
    _modifyBtn->hide();

    ml::app()->setTimeout([this]{_sessionGui.name.focus();}, 100);
}

void TimeTrackingWin::load()
{
    auto data = storage::get<json>("time-tracking-sessions");	
    this->deserializeSessions(data);
}

json TimeTrackingWin::serializeSessions() const
{
    json data = json::array();

    for (const auto& session : _sessions)
    {
        data.push_back({
            {"name", session->name},
            {"duration", session->duration},
            {"duration_done", session->duration_done},
            {"done", session->done},
        });
    }

    return data;
}

void TimeTrackingWin::deserializeSessions(const json& data)
{
    _sessions.clear();

    for (const auto& session : data)
    {
        auto s = std::make_unique<SessionData>();
        if (session.contains("name"))
            s->name = session.at("name").get<std::string>();
        if (session.contains("duration"))
            s->duration = session.at("duration").get<unsigned int>();
        if (session.contains("duration_done"))
            s->duration_done = session.at("duration_done").get<unsigned int>();
        if (session.contains("done"))
            s->done = session.at("done").get<bool>();
        _sessions.push_back(std::move(s));
    }

    this->redrawSessions();
}

void TimeTrackingWin::createCommands()
{
    auto shownewsession = [this](const std::any&){this->showNewSessionGui();};	
    auto c = _cmds.createCommand<ml::GuiCommand>("New Session", "new-session", shownewsession);
    c->setKeybind("ctrl n");

    auto showmodsession = [this](const std::any&){this->showModifySessionGui();};	
    c = _cmds.createCommand<ml::GuiCommand>("Modify Session", "show-modify-session", showmodsession);
    c->setKeybind("ctrl m");

    auto createnew = [this](const std::any&){this->createNewSession();};
    c = _cmds.createCommand<ml::GuiCommand>("Create New Session", "create-new-session", createnew);
    c->setKeybind("ctrl Return");

    auto modsession = [this](const std::any&){this->modifySession();};	
    c = _cmds.createCommand<ml::GuiCommand>("Modify Session", "modify-session", modsession);

    c = _cmds.createCommand<ml::GuiCommand>("Show Main UI", "show-main-ui", [this](const std::any&){this->showMainUI();});
    c->setKeybind("Escape");

    c = _cmds.createCommand<ml::GuiCommand>("Save", "save", [this](const std::any&){this->save();});
    c->setKeybind("ctrl s");

    c = _cmds.createCommand<ml::GuiCommand>("Reload", "reload", [this](const std::any&){this->load();});
    c->setKeybind("ctrl r");

    auto rm = [this](const std::any&){
        auto s = _fromIndex(_activeSession);
        if (!s)
        {
            ml::app()->error("No active session to remove.");
            return;
        }

        this->removeSessionFromData(s->data);
    };
    c = _cmds.createCommand<ml::GuiCommand>("Remove", "remove-session", rm);
    c->setKeybind("ctrl x");

    auto track = [this](auto&)
    {
        if (_trackIntervalId != -1)
        {
            this->stopTrack();
            return;
        }
        auto s = _fromIndex(_activeSession);
        if (!s)
        {
            ml::app()->error("No active session to track.");
            return;
        }
        
        this->startTrack(s->data);
    };
    c = _cmds.createCommand<ml::GuiCommand>("Track", "start-track-session", track);
    c->setKeybind("ctrl t");

    auto reset = [this](auto& ){
        auto s = _fromIndex(_activeSession);
        if (!s)
        {
            ml::app()->error("No active session to reset.");
            return;
        }
        
        s->data->duration_done = 0;
        s->data->done = false;
        this->redrawSessions();
    };
    c = _cmds.createCommand<ml::GuiCommand>("Reset Session", "reset-session", reset);
}

void TimeTrackingWin::createMenus()
{
    this->menu("file_tracking-sessions", "File")->addCommand(_cmds.command("new-session").get());
    this->menu("file_tracking-sessions")->addSeparator();
    this->menu("file_tracking-sessions")->addCommand(_cmds.command("reload").get());
    this->menu("file_tracking-sessions")->addCommand(_cmds.command("save").get());

    this->menu("edit_tracking-sessions", "Edit")->addCommand(_cmds.command("start-track-session").get());
    this->menu("edit_tracking-sessions", "Edit")->addCommand(_cmds.command("reset-session").get());

    this->menu("view_tracking-sessions", "View")->addCommand(_cmds.command("show-main-ui").get());
}

void TimeTrackingWin::createNewSession()
{
    auto s = std::make_unique<SessionData>();
    s->name = _sessionGui.name.value();
    s->duration = _sessionGui.duration.value() * 60 * 60;
    s->duration_done = _sessionGui.duration_done.value() * 60 * 60;
    _sessions.push_back(std::move(s));
    this->showMainUI();
}

void TimeTrackingWin::modifySession()
{
    auto s = _modifiedSession;
    s->name = _sessionGui.name.value();
    s->duration = _sessionGui.duration.value() * 60 * 60;
    s->duration_done = _sessionGui.duration_done.value() * 60 * 60;
    this->showMainUI();
}

void TimeTrackingWin::showModifySessionGui()
{
    this->stopTrack(false);
    auto s = _fromIndex(_activeSession);	
    if (!s)
    {
        ml::app()->error("No active session to modify.");
        return;
    }

    _modifiedSession = s->data;
    _sessionGui.name.set(_modifiedSession->name);
    _sessionGui.duration.set(_modifiedSession->duration / 60.0 / 60.0);
    _sessionGui.duration_done.set(_modifiedSession->duration_done / 60.0 / 60.0);
    _newSessionBox->show();
    _sessionsBox->hide();

    _createBtn->hide();
    _modifyBtn->show();
}

void TimeTrackingWin::showMainUI()
{
    _newSessionBox->hide();
    _sessionsBox->show();
    _modifiedSession = nullptr;
    this->save();
    this->redrawSessions();
}

void TimeTrackingWin::redrawSessions()
{
    _sessionsBox->clear();    	
    _sessionsDrawn.clear();
    for (auto& session : _sessions)
    {
        _drawSession(session.get());
    }

    auto s = _fromIndex(_activeSession);
    if (s)
    {
        this->setActiveSession(s);
    }
}

void TimeTrackingWin::_drawSession(SessionData* sessionData)
{
    lg("Drawing : " << sessionData->name);
    auto sd = std::make_unique<SessionDrawn>(_sessionsBox);
    sd->data = sessionData;
    sd->box = _sessionsBox->createBox().get();
    sd->box->setOrient(ml::HORIZONTAL);
    sd->box->addCssClass("session");
    if (sessionData->done)
        sd->box->addCssClass("done");
    sd->label = sd->box->createLabel(sessionData->name).get();
    sd->label->addCssClass("name");
    sd->label->setWrap(false);

    std::string dur = math::round((float)sessionData->duration_done/(60*60), 4) + "/" + math::round((float)sessionData->duration/(60*60));
    sd->duration = sd->box->createLabel(dur).get();
    sd->duration->addCssClass("duration");
    sd->duration->setWrap(false);
    auto pw = sd->box->appendProp(&sd->done);
    pw->input()->setHAlign(ml::RIGHT);

    sd->done.set(sessionData->done);

    auto ptr = sd.get();
    sd->done.addOnUpdate([this, ptr]{
                ptr->data->done = ptr->done.value(); 
                if (ptr->data->done)
                    ptr->box->addCssClass("done");
                else
                    ptr->box->removeCssClass("done");
            });

    sd->box->setContextMenu("time-session-ctx");

    auto alive = sd->alive();
    sd->box->addEventListener(ml::MOUSE_ENTER,[this, ptr, alive](auto&)
            {
                if (!alive.lock())
                    return;
                this->setActiveSession(ptr);
            });

    _sessionsDrawn.push_back(std::move(sd));
}

void TimeTrackingWin::removeSessionFromData(SessionData* sessionData)
{
    bool removed = false;
    for (auto& s : _sessions)
    {
        if (s.get() == sessionData)
        {
            _sessions.remove(s);
            removed = true;
            break;
        }
    }	

    if (removed)
    {
        this->redrawSessions();
    }
}

void TimeTrackingWin::createCtx()
{
    this->menu("time-session-ctx", "Session")->addCommand(_cmds.command("remove-session").get());	
    this->menu("time-session-ctx")->addCommand(_cmds.command("start-track-session").get());
    this->menu("time-session-ctx")->addCommand(_cmds.command("reset-session").get());
}

void TimeTrackingWin::setActiveSession(SessionDrawn* sessionDrawn)
{
    int index = _indexFromSessionDrawn(sessionDrawn);
    if (index == -1)
        return;

    _activeSession = index;

    for (auto& s : _sessionsDrawn)
        s->box->removeCssClass("active");

    if (sessionDrawn)
        sessionDrawn->box->addCssClass("active");
}

SessionDrawn* TimeTrackingWin::_fromIndex(int index)
{
    if (index < 0 || index >= _sessionsDrawn.size())
        return nullptr;
    return _sessionsDrawn[index].get();	
}

int TimeTrackingWin::_indexFromSessionDrawn(SessionDrawn* sessionDrawn)
{
    for (int i = 0; i < _sessionsDrawn.size(); i++)	
    {
        if (_sessionsDrawn[i].get() == sessionDrawn)
            return i;
    }
    return -1;
}

void TimeTrackingWin::startTrack(SessionData* sessionData)
{
    _beingTracked = _indexFromData(sessionData);
std::cout << "thread = " << std::this_thread::get_id() << std::endl;
    auto _int = [this]	
    {std::cout << "thread = " << std::this_thread::get_id() << std::endl;
        auto d = _dataFromIndex(_beingTracked); 
        if (!d)
        {
            this->stopTrack();
            return;
        }

        d->duration_done += 1;
        //d->duration_done += 1/(60*60);
        if (d->duration_done >= d->duration)
        {
            d->done = true;
            ml::app()->message("Session " + d->name + " done.");
            this->stopTrack();
        }
        this->redrawSessions();
    };
    _trackIntervalId = ml::app()->setInterval(_int, 1000);
    this->setInfos("Tracking...");
}

void TimeTrackingWin::stopTrack(bool showError )
{
    _beingTracked = -1;
    if (_trackIntervalId != -1)	
    {
        auto f = [this]
        {
            ml::app()->removeInterval(_trackIntervalId);
            _trackIntervalId = -1;
            this->setInfos("Tracking stopped.", 3000);
        };
        ml::app()->queue(f);
    }
    else if(showError)
    {
        ml::app()->error("No tracking started.");
    }
    else
    {
        lg("No tracking started, can't stop it.");
    }
}

SessionData* TimeTrackingWin::_dataFromIndex(int index)
{
    if (index < 0 || index >= _sessions.size())
        return nullptr;
    return _sessions[index].get();
}

int TimeTrackingWin::_indexFromData(SessionData* sessionData)
{
    for (int i = 0; i < _sessions.size(); i++)
    {
        if (_sessions[i].get() == sessionData)
            return i;
    }
    return -1;
}
