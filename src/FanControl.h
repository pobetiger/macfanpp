#pragma once

#include <filesystem>
#include <string>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "SMCObject.h"

// these begins with fanNN_<field>
class FanControl : public SMCObject
{
public:
    FanControl(std::filesystem::path path_, const json &SensorNames);
    // FIXME: rule of 5

    std::string PrintStatus();
    void Output(long value);
    std::string Output();
    std::string Manual();
    void Manual(bool yes);
    long Min();
    long Max();
};


