#include "./Version.h"

json Version::serialize() const
{
    json _r	;
    _r["version"] = _version;
    _r["notes"] = _notes;
    _r["data"] = _savedData;
    return _r;
}

void Version::deserialize(const json& data)
{
    if (data.contains("version"))
        _version = data["version"];
    if (data.contains("notes"))
        _notes = data["notes"];
    if (data.contains("data"))
        _savedData = data["data"];
}
