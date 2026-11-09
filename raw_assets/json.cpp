#include "raw_assets/json.h"

#include <fstream>

std::optional<nlohmann::json> LoadJson(const std::filesystem::path& path, DiagnosticSink& diagnostics)
{
    std::ifstream file(path);
    if (!file)
    {
        diagnostics.Error("Could not open settings file", path);
        return std::nullopt;
    }

    // allow_exceptions = false -> returns a "discarded" value on failure
    nlohmann::json j = nlohmann::json::parse(file, nullptr, false);
    if (j.is_discarded())
    {
        diagnostics.Error("Settings file is not valid JSON", path);
        return std::nullopt;
    }
    return j;
}

bool ReadOptional(const nlohmann::json& j, const char* key, std::optional<std::string>& out,
    const std::filesystem::path& path, DiagnosticSink& diagnostics)
{
    auto it = j.find(key);
    if (it == j.end()) return true; // Not defined, fine
    if (!it->is_string())
    {
        diagnostics.Error(std::string("Key '") + key + "' must be a string", path);
        return false;
    }
    out = it->get<std::string>();
    return true;
}