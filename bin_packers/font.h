#pragma once

#ifndef NETCOMP_FONT_H
#define NETCOMP_FONT_H

#include <filesystem>
#include <vector>

#include "raw_assets/json.h"

struct FontHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t glyphHeight;
    uint32_t glyphCount;
    uint32_t atlasWidth;
    uint32_t atlasHeight;
    uint32_t atlasChannels;
    uint32_t atlasDataSize;
};

struct FontGlyph
{
    float uvMinX;
    float uvMinY;
    float uvMaxX;
    float uvMaxY;
    uint32_t width;
    uint32_t advance;
};

struct Font
{
    FontHeader header;
    std::vector<FontGlyph> glyphs;
    std::vector<uint8_t> atlasBlob;
};

void SaveFont(const std::filesystem::path& inPath, const std::filesystem::path& outPath);
FontGlyph ParseGlyph(const nlohmann::json& e, uint32_t atlasW, uint32_t atlasH, uint32_t& outCodepoint);

#endif //NETCOMP_FONT_H