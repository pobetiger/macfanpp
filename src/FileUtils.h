
#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

std::filesystem::path FindAppleSMC();
std::string ReadFile(std::filesystem::path path);
void WriteFile(std::filesystem::path path, std::string_view data);
json ReadJsonConfig(const std::filesystem::path& filePath);

