#include "bin_packers/biome_def.h"

#include <fstream>
#include <iostream>

#include "util/magic.h"
#include "util/util.h"

std::optional<uint32_t> ParseColor(const std::string& c)
{
    if (c.size() != 7 || c[0] != '#') return std::nullopt;

    // Make sure every character after '#' is a hex digit
    for (size_t i = 1; i < c.size(); ++i)
    {
        if (!std::isxdigit(static_cast<unsigned char>(c[i]))) return std::nullopt;
    }

    return static_cast<uint32_t>(std::stoul(c.substr(1), nullptr, 16));
}

std::optional<BiomeDef> ParseBiome(const nlohmann::json& j, const LangKeys& langKeys)
{
    BiomeDef def{};

    // name -> lang index
    const std::string name = j.at("name").get<std::string>();
    const std::optional<uint32_t> nameIdx = LookupMap(langKeys, name);
    if (!nameIdx)
    {
        std::cout << "Name '" << name << "' not found in lang keys" << std::endl;
        return std::nullopt;
    }
    def.name = *nameIdx;

    // climate
    def.latTemp = j.at("lat_temp").get<float>();
    def.rain = j.at("rainfall").get<float>();

    // color
    const std::string colorStr = j.at("color").get<std::string>();
    const std::optional<uint32_t> color = ParseColor(colorStr);
    if (!color)
    {
        std::cout << "Invalid color '" << colorStr << "' (expected #rrggbb)" << std::endl;
        return std::nullopt;
    }
    def.color = *color;

    return def;
}

void SaveBiomeDefs(const std::filesystem::path& inPath, const LangKeys& langKeys, std::filesystem::path& outPath)
{
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(inPath))
        if (entry.is_regular_file() && entry.path().extension() == ".json")
            files.push_back(entry.path());
    std::ranges::sort(files);

    BiomeDefs biomes;
    bool failed = false;
    for (const auto& path : files)
    {
        bool fileOk = false;
        try
        {
            std::optional<nlohmann::json> json = LoadJson(path);
            std::optional<BiomeDef> def;
            if (json) def = ParseBiome(*json, langKeys);

            if (def) biomes.defs.push_back(*def);
            else fileOk = true;
        }
        catch (const std::exception& e) // missing keys, wrong types
        {
            std::cout << e.what() << std::endl;
            fileOk = true;
        }

        if (!fileOk)
        {
            std::cout << "Error in biome '" << path.filename().string() << "'" << std::endl;
            failed = true;
        }
    }

    if (failed)
    {
        std::cout << "Biome defs not saved because of errors" << std::endl;
        return;
    }

    biomes.header.magic      = MakeMagic('B', 'I', 'O', 'M');
    biomes.header.version    = 1;
    biomes.header.biomeCount = static_cast<uint32_t>(biomes.defs.size());

    std::ofstream out(outPath, std::ios::binary | std::ios::trunc);
    if (!out)
    {
        std::cout << "Failed to open file: " << outPath << std::endl;
        return;
    }

    out.write(reinterpret_cast<const char*>(&biomes.header), sizeof(BiomeHeader));
    out.write(reinterpret_cast<const char*>(biomes.defs.data()), static_cast<std::streamsize>(biomes.defs.size() * sizeof(BiomeDef)));

    if (!out)
    {
        std::cout << "Write failed for " << outPath.string() << std::endl;
        return;
    }

    std::cout << "Saved " << biomes.defs.size() << " biomes: " << outPath.string() << std::endl;
}