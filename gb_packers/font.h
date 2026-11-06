#pragma once

#ifndef NETCOMP_FONT_H
#define NETCOMP_FONT_H

#include <cstdint>
#include <filesystem>
#include <optional>

#include "nlohmann/json.hpp"

#include "util/vfs.h"

struct Glyph
{
    uint32_t code;
    float uvminx;
    float uvminy;
    float uvmaxx;
    float uvmaxy;
    uint32_t width;
    uint32_t advance;
};

struct Font
{
    std::string fontAtlas;
    uint32_t glyphHeight;
    std::vector<Glyph> glyphs;
};

struct FontPakHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t glyphHeight;
    uint32_t glyphCount;
    uint32_t atlasWidth;
    uint32_t atlasHeight;
    uint32_t atlasChannels;
    uint64_t atlasDataSize;
};

void from_json(const nlohmann::json& j, Glyph& glyph);
void from_json(const nlohmann::json& j, Font& font);

void SaveFont(const Vfs& vfs, const std::filesystem::path& fontAtlasPath, const std::filesystem::path& outPath, DiagnosticSink& diagnostics);

#endif //NETCOMP_FONT_H