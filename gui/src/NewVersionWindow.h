#pragma once
#include "mlgui.2/src/Window.h"
#include "mlgui.2/src/Property.h"

class NewVersionWindow : public ml::Window
{
    public:
        NewVersionWindow(ml::App* app);
        NewVersionWindow(ml::App* app, ml::Window* parent);
        virtual ~NewVersionWindow();
        virtual void init() override;

        json data();
        void reset();

    protected : 
        ml::StringProperty _notes;
};
