//#include <nlohmann/json.hpp>
#include <iostream>
#include <filesystem>
#include <iterator>
#include <fstream>
#include <vector>

//using json = nlohmann::json;

using DirIter = std::filesystem::directory_iterator;

static constexpr std::string_view hwmon_dir {"/sys/class/hwmon"};

static std::filesystem::path OpenAppleSMC();
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


std::filesystem::path OpenAppleSMC()
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
    SMCObject(std::filesystem::path path_)
        : path(path_)
    {
    }
    // FIXME: rule of 5

    unsigned long Read()
    {
        return input;
    }

    std::filesystem::path path;
    std::string label;

    unsigned long input;
};

using SensorInput = SMCObject; // alias

// these begins with fanNN_<field>
struct FanControl : public SMCObject
{
    FanControl(std::filesystem::path path_)
        : SMCObject(path_)
    {
    }
    // FIXME: rule of 5

    void Write(unsigned long output_)
    {
        output = output_;
    }

    void SetManual(bool manual_)
    {
        manual = manual_;
    }

    // FIXME: the way these are named are not quite right
    //        as we will never want to direclty cache these things
    //        just dump them out to device (maybe)
    unsigned long output;
    unsigned int manual;

    unsigned long min;
    unsigned long max;
    unsigned long safe; // Can't read TBD
};

int main(int argc, char** argv)
{
    std::vector<SensorInput> sensors;
    std::vector<FanControl> fans;

    auto smcPath = OpenAppleSMC();

    for (const auto& file : DirIter(smcPath))
    {
        std::string filenameBase {file.path().filename()};

        if (filenameBase.starts_with("fan") && filenameBase.ends_with("label"))
        {
            auto label = ReadFile(file.path());
            std::cout << "INFO : Found fan control: " << filenameBase << ", label: [" << label << "]" << std::endl;
            // TODO: filter out based on user.json
            fans.emplace_back(file.path());
        }
        else if (filenameBase.starts_with("temp") && filenameBase.ends_with("label"))
        {
            auto label = ReadFile(file.path());
            std::cout << "INFO : Found input sensor: " << filenameBase << ", label: [" << label << "] " << std::endl;
            // TODO: filter out based on user.json
            sensors.emplace_back(file.path());
        }
        // other stuff ignored
    }

    return 0;
}

