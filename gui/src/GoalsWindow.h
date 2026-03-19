#pragma once
#include "mlgui.2/src/Window.h"
#include "mlgui.2/src/Property.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace ml
{
    class Tabs;
    class TabButton;
    class Box;
    class Label;
}

struct LifeGoalProps
{
    ml::StringProperty context; 
    ml::StringProperty preflightsQuestions;
    ml::StringProperty goalsFilers;
    ml::StringProperty goals;
    bool validated = false;
    int64_t lastValidated = 0;

    LifeGoalProps();
};

class GoalsWindow : public ml::Window
{
    public : 
        GoalsWindow(ml::App* app);
        GoalsWindow(ml::App* app, ml::Window* parent);
        virtual ~GoalsWindow();

        virtual void init() override;

        void reset();
        void save();
        void read();

        json data();
        void readData(const json& data);

        std::string lifeGoalsPrompt();
        void copyLifeGoalsPromptInClipboard();

        //T is a sruct like LifeGoalProps
        template<typename T>
            void validate(T& t, ml::Box* box)
            {
                t.validated = true;
                t.lastValidated = ml::time::now();
                box->disable();
            }

    protected : 
        void _setEvents();
        
        void _createLifeGoalsTab();
        void _createYearlyTab();
        void _create90dTab();
        void _create30dTab();
        void _create7dTab();

        void _createCmds();
        void _createMenus();

        std::unique_ptr<ml::Tabs> _tabs;
        ml::TabButton* _lifeTabButton = nullptr;
        ml::TabButton* _yearTabButton = nullptr;
        ml::TabButton* _90dTabButton = nullptr;
        ml::TabButton* _30dTabButton = nullptr;
        ml::TabButton* _7dTabButton = nullptr;

        LifeGoalProps _lifeGoalsProps;

};
