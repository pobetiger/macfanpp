
#include "FileUtils.h"

#include <fstream>
#include <iostream>

using DirIter = std::filesystem::directory_iterator;

static constexpr std::string_view hwmon_dir {"/sys/class/hwmon"};

void WriteFile(std::filesystem::path path, std::string_view data)
{
    std::string content {};
    std::ofstream fout {path};
    if (fout.good())
    {
        fout << data;
        fout << std::flush;
        fout.close();
    }
    else
    {
        std::cout << "ERROR: unable to write to " << path << std::endl;
    }

}

std::string ReadFile(std::filesystem::path path)
{
    std::string content {};
    std::ifstream fin {path};
    if (fin.good())
    {
        content = std::string{std::istreambuf_iterator<char>{fin}, {}};
        content.pop_back(); // there's a known newline here
    }
    else
    {
        std::cout << "ERROR: unable to read from " << path << std::endl;
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

json ReadJsonConfig(const std::filesystem::path& filePath)
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

