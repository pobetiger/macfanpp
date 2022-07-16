
#include "SMCObject.h"

#include <iostream>
#include <sstream>

#include "FileUtils.h"


SMCObject::SMCObject(std::filesystem::path path_, const json &SensorNames)
{
    label = ReadFile(path_);
    description = SensorNames.contains(label) ?  SensorNames[label] : "";

    // strip the base and store the base path for later
    auto fileName = std::string(path_.filename());
    auto baseEnd = fileName.rfind("_label");
    if (baseEnd == std::string::npos)
    {
        // probably okay...
        //throw std::runtime_error("Invalid SMC Object path");
        baseEnd = fileName.size();
    }
    std::string baseName = fileName.substr(0, baseEnd);

    basePath = path_.parent_path() / baseName;

    std::cout << "DEBUG: " << PrintStatus() << std::endl;
}

std::string SMCObject::PrintStatus()
{
    std::stringstream ss;
    ss << "SMCObject: [" << label << "]";
    //ss << " input=" << Input();
    ss << " Temp=" << std::fixed << std::setfill('0') << std::setw(3) << std::setprecision(3)
        << Value() << "\'C";
    if (description.empty())
    {
        ss << " path=" << basePath;
    }
    else
    {
        ss << " desc=[" << Description() << "]";
    }

    return ss.str();
}


std::string SMCObject::Label() const // immutable also
{
    return label;
}
std::string SMCObject::Description() const // immutable also
{
    return description;
}

std::string SMCObject::Input()
{
    return ReadFile(Get("input"));
}

double SMCObject::Value()
{
    std::string txt = Input();
    auto value = std::stod(txt);
    value /= 1000.0;
    return value;
}


std::filesystem::path SMCObject::Get(std::string item)
{
    return {basePath.string() + "_" + item};
}

