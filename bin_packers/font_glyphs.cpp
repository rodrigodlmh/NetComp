#include "bin_packers/font_glyphs.h"

#include <fstream>
#include <iostream>

#include "raw_assets/png.h"
#include "util/magic.h"

std::optional<FontGlyphs> SaveFontGlyphs(const std::filesystem::path& inPath, const std::filesystem::path& outPath)
{
    std::optional<nlohmann::json> fontJson = LoadJson(inPath);
    if (!fontJson)
    {
        std::cout << "Failed to load json: " << inPath.string() << "\n";
        return std::nullopt;
    }
    try
    {
        FontGlyphs font{};

        uint32_t width = fontJson->at("atlas_width").get<uint32_t>();
        uint32_t unknownCp = 0;
        const std::optional<FontGlyph> unknown = ParseGlyph(fontJson->at("unknown_glyph"), width, width, unknownCp);
        if (!unknown)
        {
            std::cout << "Failed to parse unknown glyph" << std::endl;
            return std::nullopt;
        }

        // First pass: parse everything and find the highest codepoint
        const auto& glyphsJson = fontJson->at("glyphs");
        std::vector<std::pair<uint32_t, FontGlyph>> parsed;
        parsed.reserve(glyphsJson.size());

        uint32_t maxCodepoint = 0;
        for (const auto& e : glyphsJson)
        {
            uint32_t cp = 0;
            std::optional<FontGlyph> g = ParseGlyph(e, width, width, cp);
            if (!g)
            {
                std::cout << "Skipping glyph" << "\n";
                continue;
            }
            maxCodepoint = std::max(maxCodepoint, cp);
            parsed.emplace_back(cp, *g);
        }

        if (maxCodepoint > 0xFFFF)
        {
            std::cout << "SaveFont: codepoint " << maxCodepoint << " too large for a dense table\n";
            return std::nullopt;
        }

        // Dense table: index == codepoint, gaps (and index 0) = unknown glyph
        font.glyphs.assign(maxCodepoint + 1, *unknown);
        for (const auto& [cp, g] : parsed) font.glyphs[cp] = g;

        // ---- Header ----
        font.header.magic         = MakeMagic('F', 'O', 'N', 'T');
        font.header.version       = 1;
        font.header.glyphHeight   = fontJson->at("glyph_height").get<uint32_t>();
        font.header.glyphCount    = static_cast<uint32_t>(font.glyphs.size());

        // ---- Write ----
        std::ofstream out(outPath, std::ios::binary | std::ios::trunc);
        if (!out)
        {
            std::cout << "Failed to open output " << outPath.string() << "\n";
            return std::nullopt;
        }

        out.write(reinterpret_cast<const char*>(&font.header), sizeof(FontHeader));
        out.write(reinterpret_cast<const char*>(font.glyphs.data()), static_cast<std::streamsize>(font.glyphs.size() * sizeof(FontGlyph)));

        if (!out)
        {
            std::cout << "Write failed for " << outPath.string() << "\n";
            return std::nullopt;
        }
        std::cout << "Saved font.bin: " << outPath.string() << "\n";
        return font;
    }
    catch (const std::exception& ex)
    {
        std::cout << "Invalid font json (" << inPath << "): " << ex.what() << "\n";
    }
    return std::nullopt;
}

std::optional<FontGlyph> ParseGlyph(const nlohmann::json& e, const uint32_t atlasW, const uint32_t atlasH, uint32_t& outCodepoint)
{
    if (!e.is_array() || e.size() != 7)
    {
        std::cout << "Glyph must be an array of 7 elements" << std::endl;
        return std::nullopt;
    }

    outCodepoint = e.at(0).get<uint32_t>();

    const float x0 = e.at(1).get<float>();
    const float y0 = e.at(2).get<float>();
    const float x1 = e.at(3).get<float>();
    const float y1 = e.at(4).get<float>();

    FontGlyph g{};
    g.uvMinX  = x0 / static_cast<float>(atlasW);
    g.uvMinY  = y0 / static_cast<float>(atlasH);
    g.uvMaxX  = x1 / static_cast<float>(atlasW);
    g.uvMaxY  = y1 / static_cast<float>(atlasH);
    g.width   = e.at(5).get<uint32_t>();
    g.advance = e.at(6).get<uint32_t>();
    return g;
}