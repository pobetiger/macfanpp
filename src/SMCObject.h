
#pragma once

#include <string>
#include <filesystem>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class SMCObject
{
public:
    SMCObject(std::filesystem::path path_, const json &SensorNames);
    // FIXME: rule of 5

    std::string PrintStatus();
    std::string Label() const; // immutable also
    std::string Description() const; // immutable also
    std::string Input();
    double Value();

protected:
    std::filesystem::path Get(std::string item);
    std::filesystem::path basePath;
    std::string label;
    std::string description;
};


