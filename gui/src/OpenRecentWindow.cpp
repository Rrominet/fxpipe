#include "./OpenRecentWindow.h"
#include "mlgui.2/src/Box.h"
#include "mlgui.2/src/Label.h"
#include "mlgui.2/src/Scrollable.h"

#include "mlgui.2/src/Box.hpp"
#include "mlgui.2/src/Scrollable.hpp"
#include "mlgui.2/src/enums.h"
#include "storage.h"

#include "./FxPipe.h"
#include "mlgui.2/src/GuiBackendCommand.h"
#include "str.h"

unsigned int recent_label_count = 0;

RecentLabel::RecentLabel(OpenRecentWindow* win, ml::Box* parent, const std::string& name, const std::string& filepath): _parent(parent), _name(name), _filepath(filepath), _win(win)
{
    this->draw();
    recent_label_count++;
}

RecentLabel::RecentLabel(OpenRecentWindow* win, ml::Box* parent,const json& data) : _parent(parent), _win(win)
{
    if (data.contains("name") && data["name"].is_string())
        _name = data["name"].get<std::string>();
    if (data.contains("filepath") && data["filepath"].is_string())
        _filepath = data["filepath"].get<std::string>();
    this->draw();	
    recent_label_count++;
}

void RecentLabel::draw()
{
    _box = _parent->createBox().get();
    _box->addCssClass("recent-label");
    _box->setCursor("pointer");
    _namelbl = _box->createLabel().get();
    _namelbl->addCssClass("name");
    _namelbl->setHExpand(true);
    _filepathlbl = _box->createLabel().get();
    _filepathlbl->addCssClass("filepath");
    _filepathlbl->setHExpand(true);
    _filepathlbl->setHAlign(ml::RIGHT);

    _namelbl->setText(_name);
    _filepathlbl->setText(_filepath);
    this->createCtx();
    _setEvents();
}

void RecentLabel::show()
{
    if (!_box)
        this->draw();
    _box->show();
}

void RecentLabel::hide()
{
    if (_box)
        _box->hide();
}

void RecentLabel::toggle()
{
    if (!_box)
        return;

    if (_box->visible())
        _box->hide();
    else
        _box->show();
}

bool RecentLabel::visible()
{
    return _box->visible();
}

void RecentLabel::_setEvents()
{
    _box->addEventListener(ml::LEFT_UP, [this](ml::EventInfos &){
                this->click();
            });	
    _box->setContextMenu(_ctx->id());
}

void RecentLabel::click()
{
    fxpipe::get()->setCurrentFile(_filepath);
    fxpipe::get()->openCmd()->execJson(json{{"filepath", _filepath}});
    fxpipe::get()->openRecentWindow()->hide();
}

bool RecentLabel::match(const std::string& search)
{
    auto name = str::clean(_name, true);	
    auto filepath = str::clean(_filepath, true);	

    if (str::contains(name, search) || str::contains(filepath, search))
        return true;
    return false;
}

void RecentLabel::createCtx()
{
    _ctx = ml::app()->menus().create("recent-label-ctx__" + std::to_string(recent_label_count)).get(); 	
    _ctx->addButton("Remove", [this]{_win->remove(this);});
}

#define PROPS _search("Search")

OpenRecentWindow::OpenRecentWindow(ml::App* app) : ml::Window(app), PROPS
{

}

OpenRecentWindow::OpenRecentWindow(ml::App* app,ml::Window* parent) : ml::Window(app, parent), PROPS
{

}

void OpenRecentWindow::init()
{
    ml::Window::init();
    _body->addCssClass("open-recent-window");
    this->setTitle("Open recent file...");
    this->setSize(760, 330);
    _main->appendProp(&_search);
    _labelsScrollable = _main->createScrollable().get();
    _labelsScrollable->content().addCssClass("open-recent-labels");
    _labelsScrollable->setVExpand(true);
    _labelsScrollable->setHExpand(true);

    _setEvents();

    this->getRecentFiles();
    this->drawRecentFiles();
}

void OpenRecentWindow::_setEvents()
{
    auto onchange = [this]{
        auto s = _search.value();
        if (s.empty())
        {
            for (auto& l : _labels)
                l->show();
            return;
        }
        else 
        {
            for (auto& l : _labels)
                l->hide();
            auto mts = this->match(s);
            for (auto l : mts)
                l->show();
        }
    };
    _search.addOnUpdate(onchange);

    auto onvalid = [this]
    {
        auto s = _search.value();
        if (s.empty())
            return;
        auto mts = this->match(s);
        if (mts.size() == 0)
            return;
        mts[0]->click();
    };
    _search.addOnValid(onvalid);
}

void OpenRecentWindow::getRecentFiles()
{
    _recentFiles = json::array();
    _recentFiles = storage::get<json>("recent-files");
}

void OpenRecentWindow::drawRecentFiles()
{
    for (const auto& f : _recentFiles)
    {
        auto lbl = std::make_unique<RecentLabel>(this, &_labelsScrollable->content(), f);
        _labels.push_back(std::move(lbl));
    }
}

ml::Vec<RecentLabel*> OpenRecentWindow::match(std::string search)
{
    search = str::clean(search, true);
    ml::Vec<RecentLabel*> _r;   	
    for (auto& l : _labels)
    {
        if (l->match(search))
            _r.push_back(l.get());
    }
    return _r;
}

void OpenRecentWindow::remove(RecentLabel* label)
{
    json _new = json::array();
    for (auto&f : _recentFiles)
    {
        if(f["filepath"] == label->filepath())
            continue;
        else 
            _new.push_back(f);
    }

    storage::set("recent-files", _new);
    _recentFiles = _new;

    label->box()->remove();	
    for (const auto& l : _labels)
    {
        if (l.get() == label)
        {
            _labels.remove(l);
            break;
        }
    }
}
