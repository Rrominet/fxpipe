#pragma once
#include "mlgui.2/src/Window.h"
#include "mlgui.2/src/Property.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;


namespace ml
{
    class Box;
    class Label;
    class Scrollable;
    class Menu;
}

class OpenRecentWindow;
class RecentLabel
{
    public: 
        RecentLabel(OpenRecentWindow* win, ml::Box* parent, const std::string& name, const std::string& filepath);
        RecentLabel(OpenRecentWindow* win, ml::Box* parent, const json& data);
        virtual ~RecentLabel() = default;

        void draw();
        void show();
        void hide();
        void toggle();
        bool visible();

        bool match(const std::string& search);

        std::string filepath()const {return _filepath;}
        void click();

        void createCtx();
        ml::Box* box() {return _box;}

    private : 
        std::string _name;
        std::string _filepath;

        ml::Box* _parent = nullptr;
        ml::Box* _box = nullptr;
        ml::Label* _namelbl = nullptr;
        ml::Label* _filepathlbl = nullptr;
        ml::Menu* _ctx = nullptr;

        OpenRecentWindow* _win = nullptr;

        void _setEvents();
};

class OpenRecentWindow : public ml::Window
{
    public:
        OpenRecentWindow(ml::App* app);
        OpenRecentWindow(ml::App* app, ml::Window* parent);
        virtual ~OpenRecentWindow() = default;
        virtual void init() override;

        void getRecentFiles();
        void drawRecentFiles();

        ml::Vec<RecentLabel*> match(std::string search);
        void remove(RecentLabel* label);

    protected : 
        ml::Vec<std::unique_ptr<RecentLabel>> _labels;
        ml::StringProperty _search;
        ml::Scrollable* _labelsScrollable = nullptr;
        
        void _setEvents();
        json _recentFiles;
};
