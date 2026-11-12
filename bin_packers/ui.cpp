#include "bin_packers/ui.h"

#include <fstream>
#include <iostream>

#include "raw_assets/json.h"
#include "raw_assets/png.h"
#include "util/magic.h"

void SaveUi(const std::filesystem::path& inPath, const std::filesystem::path& outPath)
{
    std::optional<nlohmann::ordered_json> uiJson = LoadOrderedJson(inPath);
    if (!uiJson)
    {
        std::cout << "Failed to load json: " << inPath.string() << "\n";
        return;
    }
    try
    {
        const std::filesystem::path atlasPath = inPath.parent_path() / uiJson->at("atlas").get<std::string>();
        std::optional<Png> png = ReadRgbaPng(atlasPath);
        if (!png)
        {
            std::cout << "Failed to load font atlas: " << atlasPath.string() << "\n";
        }

        Ui ui{};
        ui.atlasBlob.assign(png->data.begin(), png->data.end());

        const auto& spritesJson = uiJson->at("sprites");
        if (!spritesJson.is_object())
        {
            std::cout << "Failed to save UI: \"sprites\" is not an object\n";
            return;
        }

        ui.sprites.reserve(spritesJson.size());
        for (const auto& [name, e] : spritesJson.items())
        {
            if (!e.is_array() || e.size() != 4)
            {
                std::cout << "Failed to save UI: sprite \"" + name + "\" must be [x0, y0, x1, y1]";
                return;
            }

            UiSprite s{};
            s.uvMinX = e.at(0).get<float>() / static_cast<float>(png->width);
            s.uvMinY = e.at(1).get<float>() / static_cast<float>(png->height);
            s.uvMaxX = e.at(2).get<float>() / static_cast<float>(png->width);
            s.uvMaxY = e.at(3).get<float>() / static_cast<float>(png->height);
            ui.sprites.push_back(s);
        }

        ui.header.magic         = MakeMagic('U', 'I', 'S', 'P');
        ui.header.version       = 1;
        ui.header.spriteCount   = static_cast<uint32_t>(ui.sprites.size());
        ui.header.atlasWidth    = static_cast<uint32_t>(png->width);
        ui.header.atlasHeight   = static_cast<uint32_t>(png->height);
        ui.header.atlasChannels = static_cast<uint32_t>(png->channels);
        ui.header.atlasDataSize = static_cast<uint32_t>(ui.atlasBlob.size());

        // ---- Write ----
        std::filesystem::create_directories(outPath);
        std::filesystem::path uiBinPath = outPath / "ui.bin";
        std::ofstream out(uiBinPath, std::ios::binary | std::ios::trunc);
        if (!out)
        {
            std::cout << "Failed to open output " << uiBinPath.string() << "\n";
            return;
        }

        out.write(reinterpret_cast<const char*>(&ui.header), sizeof(UiHeader));
        out.write(reinterpret_cast<const char*>(ui.sprites.data()), static_cast<std::streamsize>(ui.sprites.size() * sizeof(UiSprite)));
        out.write(reinterpret_cast<const char*>(ui.atlasBlob.data()), static_cast<std::streamsize>(ui.atlasBlob.size()));

        if (!out)
        {
            std::cout << "Write failed for " << uiBinPath.string() << "\n";
        }
        else
        {
            std::cout << "Saved ui.bin: " << uiBinPath.string() << "\n";
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << "SaveUi: invalid ui json (" << inPath << "): " << ex.what() << "\n";
    }
}