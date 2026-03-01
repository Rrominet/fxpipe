#pragma once

#include "str.h"
#include "vec.h"
#include "./Version.h"
#include "Ret.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class Versions
{
    public : 
        Versions() = default;
        ~Versions() = default;

        void read(int quantity=10);
        void save();

        json serialize() const;

        void createNew(const json& versionData, const std::string& notes="");

    protected : 
        ml::Vec<Version> _versions; //bp cg
        std::string _directory; //bp cgs

        ml::Ret<> _initDir();

    public : 
#include "./Versions_gen.h"
};
