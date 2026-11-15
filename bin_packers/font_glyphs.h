#pragma once

#ifndef NETCOMP_FONT_GLYPHS_H
#define NETCOMP_FONT_GLYPHS_H

#include <filesystem>
#include <vector>

#include "raw_assets/json.h"

struct FontHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t glyphHeight;
    uint32_t glyphCount;
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

struct FontGlyphs
{
    FontHeader header;
    std::vector<FontGlyph> glyphs;
};

inline const FontGlyph& GetGlyph(const FontGlyphs& f, const uint32_t cp)
{
    return cp < f.glyphs.size() ? f.glyphs[cp] : f.glyphs[0]; // index 0 == unknown
}

std::optional<FontGlyphs> SaveFontGlyphs(const std::filesystem::path& inPath, const std::filesystem::path& outPath);
std::optional<FontGlyph> ParseGlyph(const nlohmann::json& e, uint32_t atlasW, uint32_t atlasH, uint32_t& outCodepoint);

#endif //NETCOMP_FONT_GLYPHS_H