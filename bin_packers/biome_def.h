#pragma once

#ifndef NETCOMP_BIOME_DEF_H
#define NETCOMP_BIOME_DEF_H

#include <cstdint>
#include <vector>
#include <string>
#include <filesystem>
#include <optional>

#include "bin_packers/lang.h"

struct BiomeHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t biomeCount;
};

struct BiomeDef
{
    uint32_t name;
    float latTemp;
    float rain;
    uint32_t color;
};

struct BiomeDefs
{
    BiomeHeader header;
    std::vector<BiomeDef> defs;
};

std::optional<uint32_t> ParseColor(const std::string& c);
std::optional<BiomeDef> ParseBiome(const nlohmann::json& j, const LangKeys& langKeys);
void SaveBiomeDefs(const std::filesystem::path& inPath, const LangKeys& langKeys, std::filesystem::path& outPath);

#endif //NETCOMP_BIOME_DEF_H