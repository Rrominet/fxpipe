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


LifeGoalProps::LifeGoalProps() : 
    context("Context", "", "What's your current life context.\nBe factual and precise."),
    preflightsQuestions("Preflights Questions", "", "Preflights Questions, anwser them the most honestly possible."),
    goalsFilers("Goals Filter", "", "Through which easy and clear filters should a life goal pass through to be accepted and prioritized or not."),
    goals("Life Goals", "", "Your acual life Goals. Not 50. 3, 5 max.\nMake them measurable.")
{}

GoalsWindow::GoalsWindow(ml::App* app) : ml::Window(app)
{

}

GoalsWindow::GoalsWindow(ml::App* app,ml::Window* parent): ml::Window(app, parent)
{

}

GoalsWindow::~GoalsWindow()
{

}

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
    cmd->setExec([this](const auto&){
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
    _lifeGoalsProps.goals.addOnUpdate(onanypropoupdate);
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
    view = _lifeTabButton->body()->appendProp(&_lifeGoalsProps.goals, ml::MULTILINE_ENTRY);
    view->input()->setSize(-1, 350);

    _lifeTabButton->body()->appendCommand(_cmds.command("validate-life-goals").get());
}

void GoalsWindow::_createYearlyTab()
{
    _yearTabButton = _tabs->createTab("Years Goals");
    _yearTabButton->body()->createLabel("Years Goals description");
}

void GoalsWindow::_create90dTab()
{
    _90dTabButton = _tabs->createTab("90 Days Goals");
    _90dTabButton->body()->createLabel("90 Days Goals description");
}

void GoalsWindow::_create30dTab()
{
    _30dTabButton = _tabs->createTab("30 Days Goals");
    _30dTabButton->body()->createLabel("30 Days Goals description");
}

void GoalsWindow::_create7dTab()
{
    _7dTabButton = _tabs->createTab("7 Days Goals");
    _7dTabButton->body()->createLabel("7 Days Goals description");
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
    _r["life-goals"]["goals"] = _lifeGoalsProps.goals.value();
    _r["life-goals"]["validate"] = _lifeGoalsProps.validated;
    _r["life-goals"]["lastValidated"] = _lifeGoalsProps.lastValidated;
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
        
        if (lg.contains("goals") && lg["goals"].is_string())
        {
            _lifeGoalsProps.goals.set(lg["goals"].get<std::string>());
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

    if(data.contains("activetab") && data["activetab"].is_number())
        _tabs->setActiveIdx(data["activetab"].get<int>());
}
