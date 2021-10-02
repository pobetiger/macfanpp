
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <nlohmann/json.hpp>

using std::operator""s;  // std::chrono::seconds
using DirIter = std::filesystem::directory_iterator;

using json = nlohmann::json;

static constexpr std::string_view hwmon_dir {"/sys/class/hwmon"};

static std::filesystem::path FindAppleSMC();
static std::string ReadFile(std::filesystem::path path);
static json ReadJsonConfig(const std::filesystem::path& filePath);
static bool IsGlobalIgnoredFan(const json &config, std::string_view label);
static bool IsGlobalIgnoredSensor(const json &config, std::string_view label);


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
        std::cout << "TRACE: Checking " << deviceNamePath << std::endl;
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

    double Value()
    {
        std::string txt = Input();
        auto value = std::stod(txt);
        value /= 1000.0;
        return value;
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
           << "Manual: " << Manual() << ", output: "
           << Output() << " [" << Min() << ", " << Max() << "]";
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

    long Min()
    {
        auto txt = ReadFile(Get("min"));
        auto value = std::stol(txt);
        return value;
    }

    long Max()
    {
        auto txt = ReadFile(Get("max"));
        auto value = std::stol(txt);
        return value;
    }
};

static json ReadJsonConfig(const std::filesystem::path& filePath)
{
    json j;
    if (auto fin = std::ifstream{filePath};
        fin.good())
    {
        fin >> j;
        std::cout << "TRACE: JsonFile: " << filePath << " read successfully" << std::endl;
    }
    else
    {
        std::cout << "WARN : JsonFile: " << filePath << " read error" << std::endl;
    }

    return j;
}

static bool IsGlobalIgnoredFan(const json &config, std::string_view label)
{
    //std::cout << "TRACE: checking if ignored fan list exist..." << std::endl;
    if (!config.contains("global_ignore_fan"))
        return false;

    const auto &global_ignore_fans = config["global_ignore_fan"];
    //std::cout << "TRACE: checking ignored fan..." << global_ignore_fans << std::endl;
    return std::any_of(std::begin(global_ignore_fans), std::end(global_ignore_fans),[&](const auto& item) {
        return label.starts_with(std::string{item});
    });
}

static bool IsGlobalIgnoredSensor(const json &config, std::string_view label)
{
    //std::cout << "TRACE: checking if ignored sensor list exist..." << std::endl;
    if (!config.contains("global_ignore_sensor"))
        return false;

    const auto &global_ignore_sensors = config["global_ignore_sensor"];
    //std::cout << "TRACE: checking ignored sensor..." << global_ignore_sensors << std::endl;
    return std::any_of(std::begin(global_ignore_sensors), std::end(global_ignore_sensors),[&](const auto& item) {
        return item == label;
    });
}

static void MonitorGroup(const json & config, std::vector<SensorInput> &sensors, std::vector<FanControl> &fans);

static void MonitorGroup(const json & config, std::vector<SensorInput> &sensors, std::vector<FanControl> &fans)
{
    std::vector<double> PValues {};

    for (const auto &rule : config["rules"])
    {
        if (!rule.contains("name") || !rule.contains("sensors"))
            continue; // skip

        bool actOnSensor = (rule.contains("method") && rule["method"] != "none");
        bool hasTempRange = (rule.contains("temp-range") && rule["temp-range"].size() == 2);
        std::vector<double> sensor_readings{};

        std::cout << "\nINFO : Sensor Group [" << rule["name"] << "]" << std::endl;
        const auto& group_sensors = rule["sensors"];
        for (const auto& group_sensor : group_sensors)
        {
            if (auto iter = std::find_if(std::begin(sensors), std::end(sensors), [&](const auto& item) {
                    return item.Label() == group_sensor;
                });
                iter != std::end(sensors))
            {
                std::cout << "INFO : " << iter->PrintStatus() << std::endl;
                sensor_readings.push_back(iter->Value());
            }
        }

        // FIXME: refactor
        double processedVal {-1.0};
        if (actOnSensor) // FIXME: rename to processSensors
        {
            if (rule["method"] == "avg")
            {
                auto sum = std::accumulate(std::begin(sensor_readings), std::end(sensor_readings), 0.0);
                sum /= static_cast<double>(sensor_readings.size());
                std::cout << "-----\nINFO : Sensor Group [" << rule["name"] << "]"
                          << " Method=" << rule["method"] << " Val=" << sum << "\'C" << std::endl;
                processedVal = sum;
            }
            else if (rule["method"] == "max")
            {
                auto iter = std::max_element(std::begin(sensor_readings), std::end(sensor_readings));
                std::cout << "-----\nINFO : Sensor Group [" << rule["name"] << "]"
                          << " Method=" << rule["method"] << " Val=" << *iter << "\'C" << std::endl;
                processedVal = *iter;
            }
            else
            {
                std::cout << "WARN : Unsupported sensor method [" << rule["method"] << "]" << std::endl;
            }

            if (hasTempRange && processedVal > 0.0)
            {
                double minTemp = rule["temp-range"][0];
                double maxTemp = rule["temp-range"][1];



                // FIXME: do the math
                double p = (processedVal - minTemp) / (maxTemp - minTemp);

                std::cout << std::fixed << std::setfill('0') << std::setw(3) << std::setprecision(3)
                          << "INFO : Temp-Range ["
                          << minTemp << "\'C, " << maxTemp << "\'C] @ " << processedVal << "\'C"
                          << ", p=" << p
                          << std::endl;

                PValues.push_back(p);
            }

        }

    }

    // FIXME: extract to another function for fan control
    long global_fan_min {0};
    if (config.contains("global_fan_min"))
    {
        global_fan_min = config["global_fan_min"];
    }

    auto maxP = std::max_element(std::begin(PValues), std::end(PValues));
    std::cout << "\nINFO : Fan Controls "
              << "Global Fan Min=" << global_fan_min << " RPM, "
              << "Ps: [";
    for (const auto& p : PValues)
    {
        std::cout << p << ", ";
    }
    std::cout << "], MaxP=" << (maxP != std::end(PValues) ? *maxP : 0.01) << std::endl;

    for (auto &fan : fans)
    {
        auto fanRange = static_cast<double>(fan.Max() - fan.Min());
        auto fanTarget = static_cast<long>(std::round((maxP != std::end(PValues) ? *maxP : 0.01) * fanRange));

        fanTarget += std::max(fan.Min(), global_fan_min);

        std::cout << "INFO : FanControl: [" << fan.Label() << "] "
           << "Manual: " << fan.Manual() << ", output: "
           << fan.Output() << " [" << fan.Min() << ", " << fan.Max() << "]"
           << ", Target=" << fanTarget << " RPM"
           << std::endl;
    }
}


int main(int, char**)
{
    auto SensorNames = ReadJsonConfig("sensor_names.json");
    auto UserConfig = ReadJsonConfig("user.json");

    std::vector<SensorInput> sensors;
    std::vector<FanControl> fans;

    auto smcPath = FindAppleSMC();

    for (const auto& file : DirIter(smcPath)) // can throw
    {
        std::string filenameBase {file.path().filename()};

        if (filenameBase.starts_with("fan") && filenameBase.ends_with("label"))
        {
            auto obj = FanControl{file.path(), SensorNames};
            if (!IsGlobalIgnoredFan(UserConfig, obj.Label()))
            {
                std::cout << "INFO : Added fan " << obj.Label() << std::endl;
                fans.push_back(std::move(obj));
            }
            else
            {
                std::cout << "INFO : Global ignore fan " << obj.Label() << std::endl;
            }

        }
        else if (filenameBase.starts_with("temp") && filenameBase.ends_with("label"))
        {
            auto obj = SensorInput{file.path(), SensorNames};
            if (!IsGlobalIgnoredSensor(UserConfig, obj.Label()))
            {
                std::cout << "INFO : Added sensor " << obj.Label() << std::endl;
                sensors.push_back(std::move(obj));
            }
            else
            {
                std::cout << "INFO : Global ignore sensor " << obj.Label() << std::endl;
            }
        }
        // other stuff ignored
    }

    unsigned int Ts {3};
    if (UserConfig.contains("sample_period_sec"))
    {
        Ts = UserConfig["sample_period_sec"];
    }
    std::cout << "INFO : Sampling period: " << Ts << " seconds" << std::endl;

    if (!UserConfig.contains("rules"))
    {
        std::cout << "ERROR: No rules specified, exiting ..." << std::endl;
        return 0;
    }

    std::atomic<bool> is_running{true};
    while (is_running)
    {
        std::cout << "\33[2J\33[1;1f\n-----" << std::endl;
        MonitorGroup(UserConfig, sensors, fans);
        std::this_thread::sleep_for(1s * Ts);
    }

    return 0;
}

