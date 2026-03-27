#include "./GoalsWindow.h"
#include "mlgui.2/src/Tabs.h"
#include "mlgui.2/src/TabButton.h"
#include "mlgui.2/src/Box.h"

#include "mlgui.2/src/Box.hpp"
#include "mlgui.2/src/Scrollable.hpp"
#include "mlgui.2/src/App.h"

#include "fxhtml.h"
#include "files.2/files.h"
#include "mlgui.2/src/enums.h"

#include "./storage.h"


std::string life_goal_prompt = "My life context right now :\n*context*\n\nMy bruttaly honest anwser on how I feel today from these questions :\n1. What have you actually built or done in your life that you’re proud of — not feelings, concrete things\n2. What’s the worst failure you’ve had in the last 3 years and what did you conclude from it\n3. What makes you viscerally angry when you see it in the world\n4. When did you last feel genuinely alive — not happy, alive\n5. What project do you keep coming back to in your head that you’ve never fully committed to and why\nMy responses :\n*preflight*\n\nI want to extract my real life goal filter through my values. I've answered the context and pre-flight questions above. Here are the rules :\n- Ask one question at a time, never more\n- Do not validate my answers unless they're genuinely earned\n- When I give a surface answer, a socially conditioned answer, or a fear-based answer — call it out explicitly and don't move forward until we hit something real\n- Watch specifically for : me switching metrics mid-conversation, me confusing coping mechanisms with values, me using abstract feel-good words instead of precise ones\n- When you see a contradiction between what I say I value and what I've actually done — point at it directly\n- The goal is not a list of values. The goal is one or two root ideas precise enough to be exclusionary — a filter that kills projects as much as it validates them\n- Be brutal. My comfort is not the objective. Accuracy is.\n\nThe last one is the most important one.\nThe discomfort IS the signal. Don't protect me from it.";

Goal::Goal(): 
    objectives("Objectives", "", "Your actual Objectives. Not 50. 3, 5 max.\nMake them measurable.")
{}

LifeGoalProps::LifeGoalProps() : Goal(),
    context("Context", "", "What's your current life context.\nBe factual and precise."),
    preflightsQuestions("Preflights Questions", "", "Preflights Questions, anwser them the most honestly possible."),
    goalsFilers("Goals Filter", "", "Through which easy and clear filters should a life goal pass through to be accepted and prioritized or not.")
{}

GoalsWindow::GoalsWindow(ml::App* app) : ml::Window(app){}
GoalsWindow::GoalsWindow(ml::App* app,ml::Window* parent): ml::Window(app, parent){}
GoalsWindow::~GoalsWindow(){}

void GoalsWindow::_createCmds()
{
    auto cmd = _cmds.createCommand<ml::GuiCommand>("Save", "save-goals");
    cmd->setExec([this](const auto&){this->save();});
    cmd->setAfter([this](const auto&){this->setInfos("Goals saved.");});
    cmd->setHelp("Save goals.");
    cmd->setKeybind("ctrl s");


    _cmds.createCommand<ml::GuiCommand>("Life Goals Help", "show-life-goals-help",
            [this](const auto&){fxhtml::open(files::execDir() + files::sep() + "html" + files::sep() + "life-goals.html", "Life Goals - How", 600, 800);});

    _cmds.createCommand<ml::GuiCommand>("Get Prompt", "copy-life-goals-prompt", 
            [this](const auto&){this->copyLifeGoalsPromptInClipboard();});

    cmd = _cmds.createCommand<ml::GuiCommand>("Validate Life Goals", "validate-life-goals");
    cmd->setNeedConfirm(true);
    cmd->setExec([this](const auto&){
            this->validate(_lifeGoalsProps, _lifeTabButton->body().get());
            this->save();
            });

    cmd = _cmds.createCommand<ml::GuiCommand>("Edit Life Goals", "edit-life-goals");
    cmd->setNeedConfirm(true);
    cmd->setExec([this](const auto&){
            this->unvalidate(_lifeGoalsProps, _lifeTabButton->body().get());
            this->save();
            });

    cmd = _cmds.createCommand<ml::GuiCommand>("Edit Yearly Goals", "edit-yearly-goals");
    cmd->setNeedConfirm(true);
    cmd->setExec([this](const auto&){
            this->unvalidate(_yearlyGoalsProps, _yearTabButton->body().get());
            this->save();
            });

    cmd = _cmds.createCommand<ml::GuiCommand>("Edit 90 Days Goals", "edit-90d-goals");
    cmd->setNeedConfirm(true);
    cmd->setExec([this](const auto&){
            this->unvalidate(_90dGoalsProps, _90dTabButton->body().get());
            this->save();
            });

    cmd = _cmds.createCommand<ml::GuiCommand>("Edit 30 Days Goals", "edit-30d-goals");
    cmd->setNeedConfirm(true);
    cmd->setExec([this](const auto&){
            this->unvalidate(_30dGoalsProps, _30dTabButton->body().get());
            this->save();
            });

    cmd = _cmds.createCommand<ml::GuiCommand>("Edit 7 Days Goals", "edit-7d-goals");
    cmd->setNeedConfirm(true);
    cmd->setExec([this](const auto&){
            this->unvalidate(_7dGoalsProps, _7dTabButton->body().get());
            this->save();
            });

    cmd = _cmds.createCommand<ml::GuiCommand>("Validate Yearly Goals", "validate-yearly-goals");
    cmd->setNeedConfirm(true);
    cmd->setExec([this](const auto&){
            this->validate(_yearlyGoalsProps, _yearTabButton->body().get());
            this->save();
            });

    cmd = _cmds.createCommand<ml::GuiCommand>("Edit Yearly Goals", "edit-yearly-goals");
    cmd->setNeedConfirm(true);
    cmd->setExec([this](const auto&){
            this->unvalidate(_yearlyGoalsProps, _yearTabButton->body().get());
            this->save();
            });

    cmd = _cmds.createCommand<ml::GuiCommand>("Validate 90 Days Goals", "validate-90d-goals");
    cmd->setNeedConfirm(true);
    cmd->setExec([this](const auto&){
            this->validate(_90dGoalsProps, _90dTabButton->body().get());
            this->save();
            });

    cmd = _cmds.createCommand<ml::GuiCommand>("Edit 90 Days Goals", "edit-90d-goals");
    cmd->setNeedConfirm(true);
    cmd->setExec([this](const auto&){
            this->unvalidate(_90dGoalsProps, _90dTabButton->body().get());
            this->save();
            });

    cmd = _cmds.createCommand<ml::GuiCommand>("Validate 30 Days Goals", "validate-30d-goals");
    cmd->setNeedConfirm(true);
    cmd->setExec([this](const auto&){
            this->validate(_30dGoalsProps, _30dTabButton->body().get());
            this->save();
            });

    cmd = _cmds.createCommand<ml::GuiCommand>("Edit 30 Days Goals", "edit-30d-goals");
    cmd->setNeedConfirm(true);
    cmd->setExec([this](const auto&){
            this->unvalidate(_30dGoalsProps, _30dTabButton->body().get());
            this->save();
            });

    cmd = _cmds.createCommand<ml::GuiCommand>("Validate 7 Days Goals", "validate-7d-goals");
    cmd->setNeedConfirm(true);
    cmd->setExec([this](const auto&){
            this->validate(_7dGoalsProps, _7dTabButton->body().get());
            this->save();
            });

    cmd = _cmds.createCommand<ml::GuiCommand>("Edit 7 Days Goals", "edit-7d-goals");
    cmd->setNeedConfirm(true);
    cmd->setExec([this](const auto&){
            this->unvalidate(_7dGoalsProps, _7dTabButton->body().get());
            this->save();
            });
}

void GoalsWindow::_createMenus()
{
    auto file = this->menu("goals-window-file", "File");
    file->addCommand(_cmds.command("save-goals").get());
    file->addSeparator();
    file->addCommand(_cmds.command("copy-life-goals-prompt").get());
    file->addSeparator();
    file->addCommand(_cmds.command("validate-life-goals").get());
    file->addCommand(_cmds.command("validate-yearly-goals").get());
    file->addCommand(_cmds.command("validate-90d-goals").get());
    file->addCommand(_cmds.command("validate-30d-goals").get());
    file->addCommand(_cmds.command("validate-7d-goals").get());
    file->addSeparator();
    file->addCommand(_cmds.command("edit-life-goals").get());
    file->addCommand(_cmds.command("edit-yearly-goals").get());
    file->addCommand(_cmds.command("edit-90d-goals").get());
    file->addCommand(_cmds.command("edit-30d-goals").get());
    file->addCommand(_cmds.command("edit-7d-goals").get());

    auto help = this->menu("goals-window-help", "Help");
    help->addCommand(_cmds.command("show-life-goals-help").get());
}

void GoalsWindow::init()
{
    Window::init();
    this->setTitle("Personnal Goals");
    this->setSize(960, 780);

    _createCmds();
    _createMenus();

    _tabs = std::make_unique<ml::Tabs>(&this->main()->content());

    _createLifeGoalsTab();
    _createYearlyTab();
    _create90dTab();
    _create30dTab();
    _create7dTab();

    _7dTabButton->setActive(true);

    _setEvents();

    this->read();
}

void GoalsWindow::reset()
{

}

void GoalsWindow::_setEvents()
{
    auto onanypropoupdate = [this]
    {
        this->setInfos("Modified (unsaved)");
    };

    _lifeGoalsProps.context.addOnUpdate(onanypropoupdate);
    _lifeGoalsProps.preflightsQuestions.addOnUpdate(onanypropoupdate);
    _lifeGoalsProps.goalsFilers.addOnUpdate(onanypropoupdate);
    _lifeGoalsProps.objectives.addOnUpdate(onanypropoupdate);
    _yearlyGoalsProps.objectives.addOnUpdate(onanypropoupdate);
    _90dGoalsProps.objectives.addOnUpdate(onanypropoupdate);
    _30dGoalsProps.objectives.addOnUpdate(onanypropoupdate);
    _7dGoalsProps.objectives.addOnUpdate(onanypropoupdate);
}

void GoalsWindow::_createLifeGoalsTab()
{
    _lifeTabButton = _tabs->createTab("Life Goals");
    auto view = _lifeTabButton->body()->appendProp(&_lifeGoalsProps.context, ml::MULTILINE_ENTRY);
    view->input()->setSize(-1, 350);
    view = _lifeTabButton->body()->appendProp(&_lifeGoalsProps.preflightsQuestions, ml::MULTILINE_ENTRY);
    view->input()->setSize(-1, 350);
    _lifeTabButton->body()->appendCommand(_cmds.command("copy-life-goals-prompt").get());
    _lifeTabButton->body()->createSeparator();
    view = _lifeTabButton->body()->appendProp(&_lifeGoalsProps.goalsFilers, ml::MULTILINE_ENTRY);
    view->input()->setSize(-1, 350);
    view = _lifeTabButton->body()->appendProp(&_lifeGoalsProps.objectives, ml::MULTILINE_ENTRY);
    view->input()->setSize(-1, 350);

    _lifeTabButton->body()->appendCommand(_cmds.command("validate-life-goals").get());
}

void GoalsWindow::_createYearlyTab()
{
    _yearTabButton = _tabs->createTab("Years Goals");
    auto view = _yearTabButton->body()->appendProp(&_yearlyGoalsProps.objectives, ml::MULTILINE_ENTRY);
    view->input()->setSize(-1, 350);
    _yearTabButton->body()->appendCommand(_cmds.command("validate-yearly-goals").get());
}

void GoalsWindow::_create90dTab()
{
    _90dTabButton = _tabs->createTab("90 Days Goals");
    auto view = _90dTabButton->body()->appendProp(&_90dGoalsProps.objectives, ml::MULTILINE_ENTRY);
    view->input()->setSize(-1, 350);
    _90dTabButton->body()->appendCommand(_cmds.command("validate-90d-goals").get());
}

void GoalsWindow::_create30dTab()
{
    _30dTabButton = _tabs->createTab("30 Days Goals");
    auto view = _30dTabButton->body()->appendProp(&_30dGoalsProps.objectives, ml::MULTILINE_ENTRY);
    view->input()->setSize(-1, 350);
    _30dTabButton->body()->appendCommand(_cmds.command("validate-30d-goals").get());
}

void GoalsWindow::_create7dTab()
{
    _7dTabButton = _tabs->createTab("7 Days Goals");
    auto view = _7dTabButton->body()->appendProp(&_7dGoalsProps.objectives, ml::MULTILINE_ENTRY);
    view->input()->setSize(-1, 350);
    _7dTabButton->body()->appendCommand(_cmds.command("validate-7d-goals").get());
}

void GoalsWindow::save()
{
    storage::set("goals", this->data());	
}

void GoalsWindow::read()
{
    auto data = storage::get<json>("goals")	;
    this->readData(data);
}

std::string GoalsWindow::lifeGoalsPrompt()
{
    auto s = life_goal_prompt;	
    s = str::replace(s, "*context*", _lifeGoalsProps.context.value());
    s = str::replace(s, "*preflight*", _lifeGoalsProps.preflightsQuestions.value());
    return s;
}

void GoalsWindow::copyLifeGoalsPromptInClipboard()
{
    ml::app()->setClipboardText(this->lifeGoalsPrompt());
    this->setInfos("Life Goals prompt copied in clipboard.\nCtrl + V to paste it.");
}

json GoalsWindow::data()
{
    json _r;	
    _r["activetab"] = _tabs->activeIdx();
    _r["life-goals"] = json::object();
    _r["life-goals"]["context"] = _lifeGoalsProps.context.value();
    _r["life-goals"]["preflights"] = _lifeGoalsProps.preflightsQuestions.value();
    _r["life-goals"]["goals-filters"] = _lifeGoalsProps.goalsFilers.value();
    _r["life-goals"]["objectives"] = _lifeGoalsProps.objectives.value();
    _r["life-goals"]["validate"] = _lifeGoalsProps.validated;
    _r["life-goals"]["lastValidated"] = _lifeGoalsProps.lastValidated;
    _r["yearly-goals"] = json::object();
    _r["yearly-goals"]["objectives"] = _yearlyGoalsProps.objectives.value();
    _r["yearly-goals"]["validate"] = _yearlyGoalsProps.validated;
    _r["yearly-goals"]["lastValidated"] = _yearlyGoalsProps.lastValidated;
    _r["90d-goals"] = json::object();
    _r["90d-goals"]["objectives"] = _90dGoalsProps.objectives.value();
    _r["90d-goals"]["validate"] = _90dGoalsProps.validated;
    _r["90d-goals"]["lastValidated"] = _90dGoalsProps.lastValidated;
    _r["30d-goals"] = json::object();
    _r["30d-goals"]["objectives"] = _30dGoalsProps.objectives.value();
    _r["30d-goals"]["validate"] = _30dGoalsProps.validated;
    _r["30d-goals"]["lastValidated"] = _30dGoalsProps.lastValidated;
    _r["7d-goals"] = json::object();
    _r["7d-goals"]["objectives"] = _7dGoalsProps.objectives.value();
    _r["7d-goals"]["validate"] = _7dGoalsProps.validated;
    _r["7d-goals"]["lastValidated"] = _7dGoalsProps.lastValidated;
    return _r;
}

void GoalsWindow::readData(const json& data)
{
    if (data.contains("life-goals") && data["life-goals"].is_object())
    {
        auto& lg = data["life-goals"];
        
        if (lg.contains("context") && lg["context"].is_string())
        {
            _lifeGoalsProps.context.set(lg["context"].get<std::string>());
        }
        
        if (lg.contains("preflights") && lg["preflights"].is_string())
        {
            _lifeGoalsProps.preflightsQuestions.set(lg["preflights"].get<std::string>());
        }
        
        if (lg.contains("goals-filters") && lg["goals-filters"].is_string())
        {
            _lifeGoalsProps.goalsFilers.set(lg["goals-filters"].get<std::string>());
        }
        
        if (lg.contains("objectives") && lg["objectives"].is_string())
        {
            _lifeGoalsProps.objectives.set(lg["objectives"].get<std::string>());
        }
        if (lg.contains("validate") && lg["validate"].is_boolean())
        {
            _lifeGoalsProps.validated = lg["validate"].get<bool>();
            if (_lifeGoalsProps.validated)
                _lifeTabButton->body()->disable();
        }
        if (lg.contains("lastValidated") && lg["lastValidated"].is_number())
        {
            _lifeGoalsProps.lastValidated = lg["lastValidated"].get<int64_t>();
        }
    }

    if (data.contains("yearly-goals") && data["yearly-goals"].is_object())
    {
        auto& yg = data["yearly-goals"];
        
        if (yg.contains("objectives") && yg["objectives"].is_string())
        {
            _yearlyGoalsProps.objectives.set(yg["objectives"].get<std::string>());
        }
        if (yg.contains("validate") && yg["validate"].is_boolean())
        {
            _yearlyGoalsProps.validated = yg["validate"].get<bool>();
            if (_yearlyGoalsProps.validated)
                _yearTabButton->body()->disable();
        }
        if (yg.contains("lastValidated") && yg["lastValidated"].is_number())
        {
            _yearlyGoalsProps.lastValidated = yg["lastValidated"].get<int64_t>();
        }
    }

    if (data.contains("90d-goals") && data["90d-goals"].is_object())
    {
        auto& ng = data["90d-goals"];
        
        if (ng.contains("objectives") && ng["objectives"].is_string())
        {
            _90dGoalsProps.objectives.set(ng["objectives"].get<std::string>());
        }
        if (ng.contains("validate") && ng["validate"].is_boolean())
        {
            _90dGoalsProps.validated = ng["validate"].get<bool>();
            if (_90dGoalsProps.validated)
                _90dTabButton->body()->disable();
        }
        if (ng.contains("lastValidated") && ng["lastValidated"].is_number())
        {
            _90dGoalsProps.lastValidated = ng["lastValidated"].get<int64_t>();
        }
    }

    if (data.contains("30d-goals") && data["30d-goals"].is_object())
    {
        auto& tg = data["30d-goals"];
        
        if (tg.contains("objectives") && tg["objectives"].is_string())
        {
            _30dGoalsProps.objectives.set(tg["objectives"].get<std::string>());
        }
        if (tg.contains("validate") && tg["validate"].is_boolean())
        {
            _30dGoalsProps.validated = tg["validate"].get<bool>();
            if (_30dGoalsProps.validated)
                _30dTabButton->body()->disable();
        }
        if (tg.contains("lastValidated") && tg["lastValidated"].is_number())
        {
            _30dGoalsProps.lastValidated = tg["lastValidated"].get<int64_t>();
        }
    }

    if (data.contains("7d-goals") && data["7d-goals"].is_object())
    {
        auto& sg = data["7d-goals"];
        
        if (sg.contains("objectives") && sg["objectives"].is_string())
        {
            _7dGoalsProps.objectives.set(sg["objectives"].get<std::string>());
        }
        if (sg.contains("validate") && sg["validate"].is_boolean())
        {
            _7dGoalsProps.validated = sg["validate"].get<bool>();
            if (_7dGoalsProps.validated)
                _7dTabButton->body()->disable();
        }
        if (sg.contains("lastValidated") && sg["lastValidated"].is_number())
        {
            _7dGoalsProps.lastValidated = sg["lastValidated"].get<int64_t>();
        }
    }

    if(data.contains("activetab") && data["activetab"].is_number())
        _tabs->setActiveIdx(data["activetab"].get<int>());
}

void GoalsWindow::checkForPeriodPassed()
{
    auto now = ml::time::now();
    if (_7dGoalsProps.lastValidated + ml::time::days(7) < now)
    {
        _7dGoalsProps.validated = false;
        _7dTabButton->body()->enable();
        ml::app()->warning("Your 7 days goals are now expired. Please assess them and update them.");
        this->show();
    }
    if (_30dGoalsProps.lastValidated + ml::time::days(30) < now)
    {
        _30dGoalsProps.validated = false;
        _30dTabButton->body()->enable();
        ml::app()->warning("Your 30 days goals are now expired. Please assess them and update them.");
        this->show();
    }
    if (_90dGoalsProps.lastValidated + ml::time::days(90) < now)
    {
        _90dGoalsProps.validated = false;
        _90dTabButton->body()->enable();
        ml::app()->warning("Your 90 days goals are now expired. Please assess them and update them.");
        this->show();
    }
    if (_yearlyGoalsProps.lastValidated + ml::time::days(365) < now)
    {
        _yearlyGoalsProps.validated = false;
        _yearTabButton->body()->enable();
        ml::app()->warning("Your yearly goals are now expired. Please assess them and update them.");
        this->show();
    }
}
