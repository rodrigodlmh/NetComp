#include "bin_packers/block_def.h"

#include <fstream>
#include <iostream>

#include "util/magic.h"
#include "util/util.h"

std::optional<Geometry> ParseGeometry(const std::string& s)
{
    if (s == "cube") return Geometry::Cube;
    return std::nullopt;
}

std::optional<uint32_t> FindTexture(const nlohmann::json& t, const std::string& key, const TextureKeys& textureKeys, bool& ok)
{
    if (!t.contains(key)) return std::nullopt;

    const std::string texName = t[key].get<std::string>();
    const std::optional<uint32_t> idx = LookupMap(textureKeys, texName);
    if (!idx)
    {
        std::cout << "Texture '" << texName << "' not found" << std::endl;
        ok = false;
    }
    return idx;
}

std::optional<BlockDef> ParseBlock(const nlohmann::json& j, const LangKeys& langKeys, const TextureKeys& textureKeys)
{
    BlockDef def{};

    // name -> lang index
    const std::string name = j.at("name").get<std::string>();
    const std::optional<uint32_t> nameIdx = LookupMap(langKeys, name);
    if (!nameIdx)
    {
        std::cout << "Name '" << name << "' not found in lang keys" << std::endl;
        return std::nullopt;
    }
    def.name = *nameIdx;

    // geometry -> enum
    const std::string geometryStr = j.value("geometry", "cube");
    const std::optional<Geometry> geometry = ParseGeometry(geometryStr);
    if (!geometry)
    {
        std::cout << "Invalid geometry '" << geometryStr << "'" << std::endl;
        return std::nullopt;
    }
    def.geometry = static_cast<uint32_t>(*geometry);

    // textures -> layer indices
    const nlohmann::json& t = j.at("textures");
    std::array<std::optional<uint32_t>, FaceCount> faces{};
    bool ok = true;

    // All < Sides < specific
    const std::optional<uint32_t> all = FindTexture(t, "all", textureKeys, ok);
    if (all) faces.fill(*all);

    const std::optional<uint32_t> sides = FindTexture(t, "sides", textureKeys, ok);
    if (sides) faces[FaceNorth] = faces[FaceSouth] = faces[FaceEast] = faces[FaceWest] = *sides;

    static const std::pair<const char*, Face> specific[] = {
        {"up", FaceUp}, {"down", FaceDown}, {"north", FaceNorth},
        {"south", FaceSouth}, {"east", FaceEast}, {"west", FaceWest},
    };
    for (const auto& [key, face] : specific)
    {
        const std::optional<uint32_t> idx = FindTexture(t, key, textureKeys, ok);
        if (idx) faces[face] = *idx;
    }

    if (!ok) return std::nullopt;

   // Final check
    for (size_t i = 0; i < FaceCount; ++i)
    {
        if (!faces[i])
        {
            std::cout << "No texture assigned for face " << i << std::endl;
            return std::nullopt;
        }
        def.faceTextures[i] = *faces[i];
    }
    return def;
}

void SaveBlockDefs(const std::filesystem::path& inPath, const LangKeys& langKeys, const TextureKeys& textureKeys, std::filesystem::path& outPath)
{
    // Sorted so block indices are stable between runs
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(inPath))
        if (entry.is_regular_file() && entry.path().extension() == ".json")
            files.push_back(entry.path());
    std::ranges::sort(files);

    BlockDefs blocks;
    bool failed = false;
    for (const auto& path : files)
    {
        bool fileOk = false;
        try
        {
            std::optional<nlohmann::json> json = LoadJson(path);
            std::optional<BlockDef> def;
            if (json) def = ParseBlock(*json, langKeys, textureKeys);

            if (def) blocks.defs.push_back(*def);
            else fileOk = true;
        }
        catch (const std::exception& e) // missing keys, wrong types
        {
            std::cout << e.what() << std::endl;
            fileOk = true;
        }

        if (!fileOk)
        {
            std::cout << "Error in block '" << path.filename().string() << "'" << std::endl;
            failed = true;
        }
    }

    if (failed)
    {
        std::cout << "Block defs not saved because of errors" << std::endl;
        return;
    }

    blocks.header.magic      = MakeMagic('B', 'L', 'K', 'S');
    blocks.header.version    = 1;
    blocks.header.blockCount = static_cast<uint32_t>(blocks.defs.size());

    std::ofstream out(outPath, std::ios::binary | std::ios::trunc);
    if (!out)
    {
        std::cout << "Failed to open file: " << outPath << "\n";
        return;
    }

    out.write(reinterpret_cast<const char*>(&blocks.header), sizeof(BlockHeader));
    out.write(reinterpret_cast<const char*>(blocks.defs.data()), static_cast<std::streamsize>(blocks.defs.size() * sizeof(BlockDef)));

    if (!out)
    {
        std::cout << "Write failed for " << outPath.string() << "\n";
        return;
    }
    std::cout << "Saved " << blocks.defs.size() << " blocks: " << outPath.string() << "\n";
}