#pragma once

#ifndef NETCOMP_GLOBAL_SETTINGS_H
#define NETCOMP_GLOBAL_SETTINGS_H

#include <filesystem>

#include "util/vfs.h"
#include "util/diagnostics.h"

struct GlobalSettings
{
    std::filesystem::path ui_data;
    std::filesystem::path font_data;
    std::filesystem::path ui_vertex_shader;
    std::filesystem::path ui_fragment_shader;
    std::filesystem::path opaque_chunk_vertex_shader;
    std::filesystem::path opaque_chunk_fragment_shader;
    std::filesystem::path chunk_compute_shader;
};

std::optional<GlobalSettings> GetGlobalSettings(const std::filesystem::path& root, const Vfs& vfs, DiagnosticSink& diagnostics);
bool CheckExistenceAndAssign(const Vfs& vfs, std::string_view settingName, std::string_view assetId,
                             const std::filesystem::path& root, std::filesystem::path& out,
                             DiagnosticSink& diagnostics);

#endif //NETCOMP_GLOBAL_SETTINGS_H