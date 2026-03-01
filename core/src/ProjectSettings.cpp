#include "./ProjectSettings.h"


json VersioningSystem::serialize() const
{
    json _r;
    _r["type"] = type;
    _r["commands"] = commands;
    return _r;
}

void VersioningSystem::deserialize(const json& data)
{
    if (data.contains("type"))
        type = data["type"].get<Type>();	
    if (data.contains("commands"))
        commands = data["commands"].get<std::string>();
}

json ProjectSettings::serialize() const
{
    json _r;
    _r["name"] = name;
    _r["description"] = description;
    _r["versioningSystem"] = versioningSystem.serialize();
    return _r;
}

void ProjectSettings::deserialize(const json& data)
{
    if (data.contains("name"))
        name = data["name"].get<std::string>();
    if (data.contains("description"))
        description = data["description"].get<std::string>();
    if (data.contains("versioningSystem"))
        versioningSystem.deserialize(data["versioningSystem"]);
}
