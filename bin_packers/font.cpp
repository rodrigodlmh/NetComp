#include "bin_packers/font.h"

#include <fstream>
#include <iostream>

#include "raw_assets/png.h"
#include "util/magic.h"

void SaveFont(const std::filesystem::path& inPath, const std::filesystem::path& outPath)
{
    std::optional<nlohmann::json> fontJson = LoadJson(inPath);
    if (!fontJson)
    {
        std::cout << "Failed to load json: " << inPath.string() << "\n";
        return;
    }
    try
    {
        const std::filesystem::path atlasPath = inPath.parent_path() / fontJson->at("atlas").get<std::string>();
        std::optional<Png> png = ReadAlphaPng(atlasPath);
        if (!png)
        {
            std::cout << "Failed to load font atlas: " << atlasPath.string() << "\n";
        }


        Font font{};
        font.atlasBlob.assign(png->data.begin(), png->data.end());

        uint32_t unknownCp = 0;
        const FontGlyph unknown = ParseGlyph(fontJson->at("unknown_glyph"), png->width, png->height, unknownCp);

        // First pass: parse everything and find the highest codepoint
        const auto& glyphsJson = fontJson->at("glyphs");
        std::vector<std::pair<uint32_t, FontGlyph>> parsed;
        parsed.reserve(glyphsJson.size());

        uint32_t maxCodepoint = 0;
        for (const auto& e : glyphsJson)
        {
            uint32_t cp = 0;
            FontGlyph g = ParseGlyph(e, png->width, png->height, cp);
            maxCodepoint = std::max(maxCodepoint, cp);
            parsed.emplace_back(cp, g);
        }

        if (maxCodepoint > 0xFFFF)
        {
            std::cout << "SaveFont: codepoint " << maxCodepoint << " too large for a dense table\n";
            return;
        }

        // Dense table: index == codepoint, gaps (and index 0) = unknown glyph
        font.glyphs.assign(maxCodepoint + 1, unknown);
        for (const auto& [cp, g] : parsed)
            font.glyphs[cp] = g;

        // ---- Header ----
        font.header.magic         = MakeMagic('F', 'O', 'N', 'T');
        font.header.version       = 1;
        font.header.glyphHeight   = fontJson->at("glyph_height").get<uint32_t>();
        font.header.glyphCount    = static_cast<uint32_t>(font.glyphs.size());
        font.header.atlasWidth    = static_cast<uint32_t>(png->width);
        font.header.atlasHeight   = static_cast<uint32_t>(png->height);
        font.header.atlasChannels = static_cast<uint32_t>(png->channels);
        font.header.atlasDataSize = static_cast<uint32_t>(font.atlasBlob.size());

        // ---- Write ----
        std::filesystem::create_directories(outPath);
        std::filesystem::path fontBinPath = outPath / "font.bin";
        std::ofstream out(fontBinPath, std::ios::binary | std::ios::trunc);
        if (!out)
        {
            std::cout << "Failed to open output " << fontBinPath.string() << "\n";
            return;
        }

        out.write(reinterpret_cast<const char*>(&font.header), sizeof(FontHeader));
        out.write(reinterpret_cast<const char*>(font.glyphs.data()), static_cast<std::streamsize>(font.glyphs.size() * sizeof(FontGlyph)));
        out.write(reinterpret_cast<const char*>(font.atlasBlob.data()), static_cast<std::streamsize>(font.atlasBlob.size()));

        if (!out)
        {
            std::cout << "Write failed for " << fontBinPath.string() << "\n";
        }
        else
        {
            std::cout << "Saved font.bin: " << fontBinPath.string() << "\n";
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Invalid font json (" << inPath << "): " << ex.what() << "\n";
    }
}

FontGlyph ParseGlyph(const nlohmann::json& e, const uint32_t atlasW, const uint32_t atlasH, uint32_t& outCodepoint)
{
    if (!e.is_array() || e.size() != 7)
    {
        std::cout << "Glyph must be an array of 7 elements" << std::endl;
        return{};
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