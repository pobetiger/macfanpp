
#include <filesystem>
#include <fstream>
#include <iterator>
#include <iostream>
#include <sstream>
#include <vector>

#include <nlohmann/json.hpp>

using DirIter = std::filesystem::directory_iterator;

using json = nlohmann::json;

static constexpr std::string_view hwmon_dir {"/sys/class/hwmon"};

static std::filesystem::path FindAppleSMC();
static std::string ReadFile(std::filesystem::path path);

static std::string ReadFile(std::filesystem::path path)
{
    std::string content {};
    std::ifstream fin {path};
    if (fin.good())
    {
        content = std::string{std::istreambuf_iterator<char>{fin}, {}};
        content.pop_back(); // there's a known newline here
    }

    return content;
}


std::filesystem::path FindAppleSMC()
{
    // the applesmc typically looks like /sys/class/hwmon/hwmonNN/device/name
    // and the file `name` it self contains "applesmc"
    std::filesystem::path smcPath {};
    for (const auto& dirEntry : DirIter(hwmon_dir))
    {
        std::filesystem::path deviceNamePath {dirEntry.path()};
        deviceNamePath  /= "device";
        deviceNamePath  /= "name";
        std::cout << "DEBUG: Checking " << deviceNamePath << std::endl;
        if (std::filesystem::exists(deviceNamePath))
        {
            // FIXME: replace this with helper function
            std::cout << "TRACE: Device has name, checking name" << std::endl;
            std::ifstream fin {deviceNamePath};
            if (fin.good())
            {
                std::string name {std::istreambuf_iterator<char>{fin}, {}};
                name.pop_back(); // there's a known newline here
                std::cout << "TRACE: Device name: [" << name << "]" << std::endl;
                if (name == "applesmc")
                    smcPath = dirEntry.path() / "device";
            }
        }
    }

    if (!smcPath.empty())
    {
        std::cout << "INFO : Found applesmc device: " << smcPath << std::endl;
    }

    return smcPath;
}

struct SMCObject
{
    SMCObject(std::filesystem::path path_, const json &SensorNames)
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
    // FIXME: rule of 5

    std::string PrintStatus()
    {
        std::stringstream ss;
        ss << "SMCObject: [" << label << "]";
        ss << " input=" << Input();
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

    std::string Label() const // immutable also
    {
        return label;
    }
    std::string Description() const // immutable also
    {
        return description;
    }

    std::string Input()
    {
        return ReadFile(Get("input"));
    }

    std::filesystem::path Get(std::string item)
    {
        return {basePath.string() + "_" + item};
    }

    std::filesystem::path basePath;
    std::string label;
    std::string description;
};

using SensorInput = SMCObject; // alias

// these begins with fanNN_<field>
struct FanControl : public SMCObject
{
    FanControl(std::filesystem::path path_, const json &SensorNames)
        : SMCObject(path_, SensorNames)
    {
        std::cout << "DEBUG: " << PrintStatus() << std::endl;
    }
    // FIXME: rule of 5

    std::string PrintStatus()
    {
        std::stringstream ss;

        ss << "FanControl: [" << label << "] "
           << "Manual: " << Manual() << ", output: " << Output() << " [" << Min() << ", " << Max() << "]";
        return ss.str();
    }

    std::string Output()
    {
        return ReadFile(Get("output"));
    }

    std::string Manual()
    {
        return ReadFile(Get("manual"));
    }

    std::string Min()
    {
        return ReadFile(Get("min"));
    }

    std::string Max()
    {
        return ReadFile(Get("max"));
    }
};

int main(int argc, char** argv)
{
    std::ifstream fin {"sensor_names.json"};
    json SensorNames;
    fin >> SensorNames;


    std::vector<SensorInput> sensors;
    std::vector<FanControl> fans;

    auto smcPath = FindAppleSMC();

    for (const auto& file : DirIter(smcPath)) // can throw
    {
        std::string filenameBase {file.path().filename()};

        if (filenameBase.starts_with("fan") && filenameBase.ends_with("label"))
        {
            auto obj = FanControl{file.path(), SensorNames};
            // TODO: filter out based on user.json
            fans.push_back(std::move(obj));
        }
        else if (filenameBase.starts_with("temp") && filenameBase.ends_with("label"))
        {
            auto obj = SensorInput{file.path(), SensorNames};
            // TODO: filter out based on user.json
            sensors.push_back(std::move(obj));
        }
        // other stuff ignored
    }

    return 0;
}

