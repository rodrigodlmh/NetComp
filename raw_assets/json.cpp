#include "raw_assets/json.h"

#include <fstream>

std::optional<nlohmann::json> LoadJson(const std::filesystem::path& path)
{
    std::ifstream file(path);
    if (!file) return std::nullopt;
    
    nlohmann::json j = nlohmann::json::parse(file, nullptr, false);
    if (j.is_discarded()) return std::nullopt;
    return j;
}