#pragma once

#ifndef NETCOMP_FONT_H
#define NETCOMP_FONT_H

#include <cstdint>
#include <filesystem>

#include "nlohmann/json.hpp"

#include "util/vfs.h"

struct JsonGlyph
{
    uint32_t code;
    float uvminx;
    float uvminy;
    float uvmaxx;
    float uvmaxy;
    uint32_t width;
    uint32_t advance;
};

struct JsonFont
{
    std::string fontAtlas;
    uint32_t glyphHeight;
    JsonGlyph unknownGlyph;
    std::vector<JsonGlyph> glyphs;
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

struct FontPakGlyph // Doesn't include the code since the code is the index to the array
{
    float uvminx;
    float uvminy;
    float uvmaxx;
    float uvmaxy;
    uint32_t width;
    uint32_t advance;
};

struct FontPak
{
    FontPakHeader header;
    std::vector<FontPakGlyph> glyphs;
    std::vector<uint8_t> atlasData;
};

void from_json(const nlohmann::json& j, JsonGlyph& glyph);
void from_json(const nlohmann::json& j, JsonFont& font);

void SaveFont(const Vfs& vfs, const std::filesystem::path& jsonPath,
              const std::filesystem::path& outPath, DiagnosticSink& diagnostics);

#endif //NETCOMP_FONT_H