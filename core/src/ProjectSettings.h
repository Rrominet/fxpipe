#pragma once
#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct VersioningSystem
{
    enum Type
    {
        GLOBAL = 0,
        PER_ASSET,
    };
    
    Type type = GLOBAL;

    json serialize() const;
    void deserialize(const json& data);

    //executed when a new version is created
    std::string commands;
};

struct ProjectSettings
{
    std::string name;
    std::string description;
    VersioningSystem versioningSystem;

    json serialize() const;
    void deserialize(const json& data);
};
