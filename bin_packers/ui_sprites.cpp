#include "bin_packers/ui_sprites.h"

#include <fstream>
#include <iostream>

#include "raw_assets/json.h"
#include "raw_assets/png.h"
#include "util/magic.h"

void SaveUiSprites(const std::filesystem::path& inPath, const std::filesystem::path& outPath)
{
    std::optional<nlohmann::ordered_json> uiJson = LoadOrderedJson(inPath);
    if (!uiJson)
    {
        std::cout << "Failed to load json: " << inPath.string() << "\n";
        return;
    }
    try
    {
        UiSprites ui{};

        uint32_t width = uiJson->at("atlas_width").get<uint32_t>();
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
            s.uvMinX = e.at(0).get<float>() / static_cast<float>(width);
            s.uvMinY = e.at(1).get<float>() / static_cast<float>(width);
            s.uvMaxX = e.at(2).get<float>() / static_cast<float>(width);
            s.uvMaxY = e.at(3).get<float>() / static_cast<float>(width);
            ui.sprites.push_back(s);
        }

        ui.header.magic         = MakeMagic('U', 'I', 'S', 'P');
        ui.header.version       = 1;
        ui.header.spriteCount   = static_cast<uint32_t>(ui.sprites.size());

        // ---- Write ----
        std::ofstream out(outPath, std::ios::binary | std::ios::trunc);
        if (!out)
        {
            std::cout << "Failed to open output " << outPath.string() << "\n";
            return;
        }

        out.write(reinterpret_cast<const char*>(&ui.header), sizeof(UiHeader));
        out.write(reinterpret_cast<const char*>(ui.sprites.data()), static_cast<std::streamsize>(ui.sprites.size() * sizeof(UiSprite)));

        if (!out)
        {
            std::cout << "Write failed for " << outPath.string() << "\n";
        }
        else
        {
            std::cout << "Saved ui.bin: " << outPath.string() << "\n";
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << "SaveUi: invalid ui json (" << inPath << "): " << ex.what() << "\n";
    }
}