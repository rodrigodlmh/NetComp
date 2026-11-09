#pragma once

#ifndef NETCOMP_SETTINGS_H
#define NETCOMP_SETTINGS_H

#include <filesystem>
#include <map>

#include "util/vfs.h"
#include "util/diagnostics.h"

namespace Defs
{
    inline constexpr const char* UiData       = "ui_data";
    inline constexpr const char* FontData     = "font_data";
    inline constexpr const char* ChunkCompute = "chunk_compute_shader";
}

using Definitions = std::map<std::string, std::string>;

struct PackSettings
{
    std::string packName;
    Definitions definitions;
};

// Global settings for all packs
struct GlobalSettings
{
    std::vector<std::string> packs;
};

std::optional<std::string> GetDef(const Definitions& defs, const std::string& name);
std::optional<PackSettings> GetLocalSettings(const std::filesystem::path& root, DiagnosticSink& diagnostics);
void OverrideDefinitions(Definitions& defs, std::optional<PackSettings> other);
std::optional<GlobalSettings> GetGlobalSettings(const std::filesystem::path& root, DiagnosticSink& diagnostics);

#endif //NETCOMP_SETTINGS_H