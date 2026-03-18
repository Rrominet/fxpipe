#include "./GoalsWindow.h"
#include "mlgui.2/src/Tabs.h"
#include "mlgui.2/src/TabButton.h"
#include "mlgui.2/src/Box.h"

#include "mlgui.2/src/Box.hpp"
#include "mlgui.2/src/Scrollable.hpp"

GoalsWindow::GoalsWindow(ml::App* app) : ml::Window(app)
{

}

GoalsWindow::GoalsWindow(ml::App* app,ml::Window* parent): ml::Window(app, parent)
{

}

GoalsWindow::~GoalsWindow()
{

}

void GoalsWindow::init()
{
    Window::init();
    this->setTitle("Personnal Goals");
    this->setSize(960, 480);

    _tabs = std::make_unique<ml::Tabs>(&this->main()->content());

    _createLifeGoalsTab();
    _createYearlyTab();
    _create90dTab();
    _create30dTab();
    _create7dTab();

    _7dTabButton->setActive(true);

    _setEvents();
}

void GoalsWindow::reset()
{

}

void GoalsWindow::_setEvents()
{

}

void GoalsWindow::_createLifeGoalsTab()
{
    _lifeTabButton = _tabs->createTab("Life Goals");
    _lifeTabButton->body()->createLabel("Life Goals description");
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
	
}

void GoalsWindow::read()
{
	
}
