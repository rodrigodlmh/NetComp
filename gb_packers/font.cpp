#include "gb_packers/font.h"

#include <fstream>

#include "raw_assets/png.h"
#include "util/magic.h"

void from_json(const nlohmann::json& j, Glyph& glyph)
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

void from_json(const nlohmann::json& j, Font& font)
{
    font.fontAtlas = j.at("texture").get<std::string>();
    font.glyphHeight = j.at("glyph_height").get<uint32_t>();
    font.glyphs = j.at("glyphs").get<std::vector<Glyph>>();
}

void SaveFont(const Vfs& vfs, const std::filesystem::path& path, const std::filesystem::path& outPath)
{
    // Read json
    std::ifstream json(path);
    if (!json.is_open()) return;

    nlohmann::json j;
    json >> j;
    auto font = j.get<Font>();

    // Check out path
    std::ofstream file(outPath, std::ios::binary);
    if (!file.is_open()) return;

    std::optional<std::filesystem::path> atlasPath = vfs.GetAsset(font.fontAtlas);
    if (!atlasPath.has_value()) return;
    Png png = ReadPng(atlasPath.value());

    for (auto& glyph : font.glyphs)
    {
        glyph.uvminx /= static_cast<float>(png.width);
        glyph.uvminy /= static_cast<float>(png.height);
        glyph.uvmaxx /= static_cast<float>(png.width);
        glyph.uvmaxy /= static_cast<float>(png.height);
    }

    FontPakHeader header{};
    header.magic = MakeMagic('F', 'P', 'A', 'K');
    header.version = 1;
    header.glyphHeight = font.glyphHeight;
    header.glyphCount = static_cast<uint32_t>(font.glyphs.size());
    header.atlasWidth = png.width;
    header.atlasHeight = png.height;
    header.atlasChannels = png.channels;
    header.atlasDataSize = png.data.size();

    file.write(reinterpret_cast<const char*>(&header), sizeof(FontPakHeader));
    file.write(reinterpret_cast<const char*>(font.glyphs.data()), font.glyphs.size() * sizeof(Glyph));
    file.write(reinterpret_cast<const char*>(png.data.data()), png.data.size() * sizeof(png.data[0]));
}