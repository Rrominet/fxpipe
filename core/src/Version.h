#pragma once
#include "str.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class Version
{
    public: 
        Version() = default;
        Version(int version, std::string notes) : _version(version), _notes(notes) {}
        Version(const json& data) { this->deserialize(data); }
        ~Version() = default;

        void increment(){_version++;}

        json serialize() const;
        void deserialize(const json& data);

        std::string paddedVersion() const {return str::pad(_version, 10, '0');}

    private : 
        int _version = 0;     //bp cgs
        std::string _notes;   //bp cgs
        json _savedData;      //bp cgs
        
    public : 
#include "./Version_gen.h"
    
};
