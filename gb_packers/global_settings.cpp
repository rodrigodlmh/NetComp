#include "gb_packers/global_settings.h"

#include <fstream>

#include "nlohmann/json.hpp"

std::optional<GlobalSettings> GetGlobalSettings(const std::filesystem::path& root, const Vfs& vfs, DiagnosticSink& diagnostics)
{
    std::ifstream file(root);
    if (!file)
    {
        diagnostics.Error("Could not open global settings file", root);
        return std::nullopt;
    }

    nlohmann::json j;
    try
    {
        j = nlohmann::json::parse(file);
    }
    catch (const nlohmann::json::exception& e)
    {
        diagnostics.Error("Invalid global settings JSON: " + std::string(e.what()), root);
        return std::nullopt;
    }

    GlobalSettings settings{};
    try
    {
        bool success = true;

        success &= CheckExistenceAndAssign(vfs, "ui_data",
        j.at("ui_data").get<std::string>(),
            root, settings.ui_data, diagnostics);

        success &= CheckExistenceAndAssign(
            vfs, "font_data",
            j.at("font_data").get<std::string>(),
            root, settings.font_data, diagnostics);

        success &= CheckExistenceAndAssign(
            vfs, "ui_vertex_shader",
            j.at("ui_vertex_shader").get<std::string>(),
            root, settings.ui_vertex_shader, diagnostics);

        success &= CheckExistenceAndAssign(
            vfs, "ui_fragment_shader",
            j.at("ui_fragment_shader").get<std::string>(),
            root, settings.ui_fragment_shader, diagnostics);

        success &= CheckExistenceAndAssign(
            vfs, "opaque_chunk_vertex_shader",
            j.at("opaque_chunk_vertex_shader").get<std::string>(),
            root, settings.opaque_chunk_vertex_shader, diagnostics);

        success &= CheckExistenceAndAssign(
            vfs, "opaque_chunk_fragment_shader",
            j.at("opaque_chunk_fragment_shader").get<std::string>(),
            root, settings.opaque_chunk_fragment_shader, diagnostics);

        success &= CheckExistenceAndAssign(
            vfs, "chunk_compute_shader",
            j.at("chunk_compute_shader").get<std::string>(),
            root, settings.chunk_compute_shader, diagnostics);

        if (!success) return std::nullopt;
    }
    catch (const nlohmann::json::exception& e)
    {
        diagnostics.Error("Invalid global settings: " + std::string(e.what()), root);
        return std::nullopt;
    }

    return settings;
}

bool CheckExistenceAndAssign(const Vfs& vfs,
    const std::string_view settingName,
    const std::string_view assetId,
    const std::filesystem::path& root,
    std::filesystem::path& out,
    DiagnosticSink& diagnostics)
{
    if (const auto asset = vfs.GetAsset(assetId.data()); asset.has_value())
    {
        out = *asset;
        return true;
    }

    diagnostics.Error("'" + std::string(settingName) + "' references nonexistent asset '" + std::string(assetId) + "'", root);
    return false;
}
