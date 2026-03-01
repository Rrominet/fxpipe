#pragma once
#include "str.h"
#include "Ret.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

//simply write the changement in status in different tasks over time.
//let create stats on production and also generate the version commits message.

class Task;
class Stats
{
    public : 
        Stats() = default;
        virtual ~Stats() = default;

        //save all the stats that are open
        //each stat correspond to a day
        ml::Ret<> save();

        //quantity go from the most recent to the oldest
        ml::Ret<> read(int quantity=1);

        ml::Ret<> saveTaskStatusChange(Task* task);

    protected :
        std::string _directory; //bp cgs
        json _data; //bp cg

        ml::Ret<> _initDir();
        void _saveDate(const std::string& dateKey);
        
    public : 
#include "./Stats_gen.h"
};
