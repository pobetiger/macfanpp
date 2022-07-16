
#include <chrono>
using std::operator""s;  // std::chrono::seconds

#include <filesystem>
using DirIter = std::filesystem::directory_iterator;

#include <iterator>
#include <iostream>
#include <numeric>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "FileUtils.h"

#include "SMCObject.h"
using SensorInput = SMCObject; // alias

#include "FanControl.h"

#include "DBusAppIF.h"





static bool IsGlobalIgnoredFan(const json &config, std::string_view label);
static bool IsGlobalIgnoredSensor(const json &config, std::string_view label);

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

static std::string MonitorGroup(const json & config, std::vector<SensorInput> &sensors, std::vector<FanControl> &fans);
static std::string MonitorGroup(const json & config, std::vector<SensorInput> &sensors, std::vector<FanControl> &fans)
{
    std::vector<double> PValues {};

    json out{};
    out["program"] = {"macfanpp"};

    for (const auto &rule : config["rules"])
    {
        if (!rule.contains("name") || !rule.contains("sensors"))
            continue; // skip

        bool actOnSensor = (rule.contains("method") && rule["method"] != "none");
        bool hasTempRange = (rule.contains("temp-range") && rule["temp-range"].size() == 2);
        std::vector<double> sensor_readings{};

        //std::cout << "\nINFO : Sensor Group [" << rule["name"] << "]" << std::endl;

        json sensors_json;
        const auto& group_sensors = rule["sensors"];
        for (const auto& group_sensor : group_sensors)
        {
            if (auto iter = std::find_if(std::begin(sensors), std::end(sensors), [&](const auto& item) {
                    return item.Label() == group_sensor;
                });
                iter != std::end(sensors))
            {
                sensors_json.push_back({
                    {"label", iter->Label()},
                    {"temperature", iter->Value()},
                    {"description", iter->Description()}
                });

                //std::cout << "INFO : " << iter->PrintStatus() << std::endl;
                sensor_readings.push_back(iter->Value());
            }
        }


        // FIXME: refactor
        json sensor_processed;
        json calcuations;
        double processedVal {-1.0};
        if (actOnSensor) // FIXME: rename to processSensors
        {
            if (rule["method"] == "avg")
            {
                auto sum = std::accumulate(std::begin(sensor_readings), std::end(sensor_readings), 0.0);
                sum /= static_cast<double>(sensor_readings.size());

                sensor_processed.push_back({
                    { "avg",
                        { "value", sum }
                    }
                });

                //std::cout << "-----\nINFO : Sensor Group [" << rule["name"] << "]"
                //          << " Method=" << rule["method"] << " Val=" << sum << "\'C" << std::endl;
                processedVal = sum;
            }
            else if (rule["method"] == "max")
            {
                auto iter = std::max_element(std::begin(sensor_readings), std::end(sensor_readings));

                sensor_processed.push_back({
                    { "max", 
                        { "value", *iter }
                    }
                });

                //std::cout << "-----\nINFO : Sensor Group [" << rule["name"] << "]"
                //          << " Method=" << rule["method"] << " Val=" << *iter << "\'C" << std::endl;
                processedVal = *iter;
            }
            else
            {
                sensor_processed.push_back({
                    { rule["method"].get<std::string>(),
                        { "error", "unsupported sensor method" }
                    }
                });

                std::cout << "WARN : Unsupported sensor method [" << rule["method"] << "]" << std::endl;
            }


            if (hasTempRange && processedVal > 0.0)
            {
                double minTemp = rule["temp-range"][0];
                double maxTemp = rule["temp-range"][1];

                // FIXME: do the math
                double p = (processedVal - minTemp) / (maxTemp - minTemp);

                //std::cout << std::fixed << std::setfill('0') << std::setw(3) << std::setprecision(3)
                //          << "INFO : Temp-Range ["
                //          << minTemp << "\'C, " << maxTemp << "\'C] @ " << processedVal << "\'C"
                //          << ", p=" << p
                //          << std::endl;

                calcuations = {
                    { "temp-range", { minTemp, maxTemp }},
                    { "process-value", processedVal },
                    { "p", p }
                };

                PValues.push_back(p);
            }
        }

        out[rule["name"].get<std::string>()] = {
            { "sensors", sensors_json },
            { "processed", sensor_processed },
            { "calc", calcuations }
        };
    }

    json decision_json;

    // FIXME: extract to another function for fan control
    long global_fan_min {0};
    if (config.contains("global_fan_min"))
    {
        global_fan_min = config["global_fan_min"];
    }

    auto maxP = std::max_element(std::begin(PValues), std::end(PValues));
    //std::cout << "\nINFO : Fan Controls "
    //          << "Global Fan Min=" << global_fan_min << " RPM, "
    //          << "Ps: [";
    //for (const auto& p : PValues)
    //{
    //    std::cout << p << ", ";
    //}
    auto maxP_Print = (maxP != std::end(PValues) ? *maxP : 0.01);
    //std::cout << "], MaxP=" << maxP_Print << std::endl;

    decision_json.push_back({
        { "global-fan-min", global_fan_min },
        { "Ps", PValues },
        { "MaxP",  maxP_Print }
    });

    json actualation;
    for (auto &fan : fans)
    {
        auto fanRange = static_cast<double>(fan.Max() - fan.Min());
        auto fanTarget = static_cast<long>(std::round((maxP != std::end(PValues) ? *maxP : 0.01) * fanRange));

        fanTarget += std::max(fan.Min(), global_fan_min);
        fanTarget = std::min(fan.Max(), fanTarget);

        //std::cout << "INFO : FanControl: [" << fan.Label() << "] "
        //   << "Manual: " << fan.Manual() << ", output: "
        //   << fan.Output() << " [" << fan.Min() << ", " << fan.Max() << "]"
        //   << ", Target=" << fanTarget << " RPM"
        //   << std::endl;

        actualation.push_back({
            { "Fan", fan.Label() },
            { "Manual", fan.Manual() },
            { "Output", fan.Output() },
            { "Range", { fan.Min(), fan.Max() } },
            { "Target", fanTarget }
        });

        fan.Output(fanTarget);
    }

    out["decision"] = decision_json;
    out["actions"] = actualation;

    return out.dump();
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
                obj.Manual(true);
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

    auto app = std::make_unique<DBusAppIF>();

    std::locale::global(std::locale(""));
    DBusAppIF::InitEnv();

    app->RegisterTimeoutHandle([&]() {
        //std::cout << "\33[2J\33[1;1f\n-----" << std::endl;
        return MonitorGroup(UserConfig, sensors, fans);
        // FIXME: set time period 
        //std::this_thread::sleep_for(1s * Ts);
    });

    // starts the application loop
    app->Run();

    // FIXME: NO_DBUS is broken here

    return 0;
}

