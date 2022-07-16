
#include "FanControl.h"

#include <sstream>
#include <iostream>

#include "FileUtils.h"


// these begins with fanNN_<field>
FanControl::FanControl(std::filesystem::path path_, const json &SensorNames)
    : SMCObject(path_, SensorNames)
{
    std::cout << "DEBUG: " << PrintStatus() << std::endl;
}
// FIXME: rule of 5

std::string FanControl::PrintStatus()
{
    std::stringstream ss;

    ss << "FanControl: [" << label << "] "
        << "Manual: " << Manual() << ", output: "
        << Output() << " [" << Min() << ", " << Max() << "]";
    return ss.str();
}

void FanControl::Output(long value)
{

    std::stringstream ss;
    ss << value;

    std::string txt {ss.str()};
    //std::cout << "TRACE: write " << txt << " to " << Get("output") << std::endl;
    WriteFile(Get("output"), txt);
}

std::string FanControl::Output()
{
    return ReadFile(Get("output"));
}

std::string FanControl::Manual()
{
    return ReadFile(Get("manual"));
}

void FanControl::Manual(bool yes)
{
    WriteFile(Get("manual"), (yes ? "1" : "0"));
}

long FanControl::Min()
{
    auto txt = ReadFile(Get("min"));
    auto value = std::stol(txt);
    return value;
}

long FanControl::Max()
{
    auto txt = ReadFile(Get("max"));
    auto value = std::stol(txt);
    return value;
}
