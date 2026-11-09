#include "gb_packers/settings.h"

#include "raw_assets/json.h"

std::optional<std::string> GetDef(const Definitions& defs, const std::string& name)
{
    auto it = defs.find(name);
    return it != defs.end() ? std::optional{it->second} : std::nullopt;
}

std::optional<PackSettings> GetLocalSettings(const std::filesystem::path& root, DiagnosticSink& diagnostics)
{
    auto j = LoadJson(root, diagnostics);
    if (!j) return std::nullopt;

    PackSettings s;

    // Name
    auto nameIt = j->find("name");
    if (nameIt == j->end() || !nameIt->is_string())
    {
        s.packName = root.parent_path().filename().string();
    }
    else
    {
        s.packName = nameIt->get<std::string>();
    }

    // Definitions
    auto defs = j->find("definitions");
    if (defs == j->end()) return s;

    if (!defs->is_object())
    {
        diagnostics.Error("'definitions' must be an object", root);
        return std::nullopt;
    }

    bool ok = true;
    for (const auto& [key, value] : defs->items())
    {
        if (!value.is_string())
        {
            diagnostics.Error("Definition '" + key + "' must be a string", root);
            ok = false;
            continue;
        }
        s.definitions[key] = s.packName + ":" + value.get<std::string>();
    }

    return ok ? std::optional{std::move(s)} : std::nullopt;
}

void OverrideDefinitions(Definitions& defs, std::optional<PackSettings> other)
{
    if (!other.has_value()) return;

    for (const auto& [name, value] : other.value().definitions)
        defs.insert_or_assign(name, value);
}

std::optional<GlobalSettings> GetGlobalSettings(const std::filesystem::path& root, DiagnosticSink& diagnostics)
{
    auto j = LoadJson(root, diagnostics);
    if (!j) return std::nullopt;

    GlobalSettings settings{};
    settings.packs = j.value().at("packs").get<std::vector<std::string>>();

    return settings;
}
