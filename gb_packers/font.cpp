#include "gb_packers/font.h"

#include <fstream>

#include "raw_assets/png.h"
#include "util/magic.h"

void from_json(const nlohmann::json& j, JsonGlyph& glyph)
{
    if (!j.is_array() || j.size() != 7)
        throw nlohmann::json::type_error::create(302, "Invalid glyph definition", &j);

    glyph.code    = j[0].get<uint32_t>();
    glyph.uvminx  = j[1].get<float>();
    glyph.uvminy  = j[2].get<float>();
    glyph.uvmaxx  = j[3].get<float>();
    glyph.uvmaxy  = j[4].get<float>();
    glyph.width   = j[5].get<uint32_t>();
    glyph.advance = j[6].get<uint32_t>();
}

void from_json(const nlohmann::json& j, JsonFont& font)
{
    font.fontAtlas = j.at("texture").get<std::string>();
    font.glyphHeight = j.at("glyph_height").get<uint32_t>();
    font.unknownGlyph = j.at("unknown_glyph").get<JsonGlyph>();
    font.glyphs = j.at("glyphs").get<std::vector<JsonGlyph>>();
}

void SaveFont(const Vfs& vfs, const std::filesystem::path& jsonPath,
              const std::filesystem::path& outPath, DiagnosticSink& diagnostics)
{
 std::ifstream jsonFile(jsonPath);
    if (!jsonFile.is_open()) { diagnostics.Error("Failed to read font json", jsonPath); return; }

    JsonFont font;
    try {
        font = nlohmann::json::parse(jsonFile).get<JsonFont>();
    } catch (const nlohmann::json::exception& e) {
        diagnostics.Error(std::string("Invalid font json: ") + e.what(), jsonPath);
        return;
    }

    if (font.glyphs.empty()) { diagnostics.Error("Font has no glyphs", jsonPath); return; }

    auto atlasPath = vfs.GetAsset(font.fontAtlas);
    if (!atlasPath) { diagnostics.Error("Font atlas couldn't be found", jsonPath); return; }

    Png png = ReadAlphaPng(*atlasPath);
    if (png.width == 0 || png.height == 0) { diagnostics.Error("Invalid atlas dimensions", *atlasPath); return; }

    std::ranges::sort(font.glyphs, {}, &JsonGlyph::code);
    for (size_t i = 1; i < font.glyphs.size(); ++i)
        if (font.glyphs[i].code == font.glyphs[i - 1].code)
        { diagnostics.Error("Duplicate glyph code", jsonPath); return; }

    const uint32_t maxGlyphCode = font.glyphs.back().code;
    if (maxGlyphCode > 0x10FFFF) { diagnostics.Error("Glyph code out of range", jsonPath); return; }

    auto toPak = [&](const JsonGlyph& g) {
        return FontPakGlyph{
            g.uvminx / static_cast<float>(png.width), g.uvminy / static_cast<float>(png.height),
            g.uvmaxx / static_cast<float>(png.width), g.uvmaxy / static_cast<float>(png.height),
            g.width, g.advance };
    };

    FontPak pak{};
    pak.header.magic = MakeMagic('F','P','A','K');
    pak.header.version = 1;
    pak.header.glyphHeight = font.glyphHeight;
    pak.header.glyphCount = maxGlyphCode + 1;
    pak.header.atlasWidth = png.width;
    pak.header.atlasHeight = png.height;
    pak.header.atlasChannels = png.channels;
    pak.header.atlasDataSize = png.data.size();

    pak.glyphs.assign(maxGlyphCode + 1, toPak(font.unknownGlyph));
    for (const auto& g : font.glyphs)
        pak.glyphs[g.code] = toPak(g);

    std::ofstream file(outPath, std::ios::binary);
    if (!file.is_open()) { diagnostics.Error("Failed to open output pak", outPath); return; }

    file.write(reinterpret_cast<const char*>(&pak.header), sizeof(pak.header));
    file.write(reinterpret_cast<const char*>(pak.glyphs.data()), pak.glyphs.size() * sizeof(FontPakGlyph));
    file.write(reinterpret_cast<const char*>(png.data.data()), png.data.size());
    if (!file) diagnostics.Error("Failed writing font pak", outPath);
}