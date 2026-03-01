#include "./NewVersionWindow.h"
#include "mlgui.2/src/enums.h"
#include "./FxPipe.h"

#include "mlgui.2/src/Scrollable.hpp"

#define CONSTRUCT_PROPS \
    _notes("Notes", "", "New version notes")

NewVersionWindow::NewVersionWindow(ml::App* app) : ml::Window(app), CONSTRUCT_PROPS
{

}

NewVersionWindow::NewVersionWindow(ml::App* app,ml::Window* parent) : ml::Window(app, parent), CONSTRUCT_PROPS
{

}

NewVersionWindow::~NewVersionWindow()
{

}

void NewVersionWindow::init()
{

    ml::Window::init();
    this->setSize(550, 740);
    _main->appendProp(&_notes, ml::MULTILINE_ENTRY);
    _main->appendCommand("new-version");

    auto onres = [this](const json& res)
    {
        if (!res.contains("data"))
            return;
        _notes.set(res["data"].get<std::string>());
    };
    fxpipe::get()->executeBackendCommand(fxpipe::get()->backend(), "current-stats-done", json::object(), onres);
}

json NewVersionWindow::data()
{
    json _r;	
    _r["notes"] = _notes.value();
    return _r;
}

void NewVersionWindow::reset()
{
    _notes.set(_S"");
}
