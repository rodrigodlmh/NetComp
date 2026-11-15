#include "bin_packers/texture.h"

#include <fstream>
#include <iostream>

#include "raw_assets/png.h"
#include "util/magic.h"

void SaveTexture(const std::filesystem::path& inPath, const std::filesystem::path& outPath)
{
    std::optional<Png> png = ReadRgbaPng(inPath);
    if (!png)
    {
        std::cout << "Failed to read png: " << inPath << std::endl;
        return;
    }

    Texture texture;
    texture.header.magic = MakeMagic('T', 'E', 'X', '2');
    texture.header.version = 1;
    texture.header.width = png->width;
    texture.header.height = png->height;
    texture.header.channels = png->channels;
    texture.header.layers = 1;
    texture.header.dataSize = png->data.size();
    texture.data.assign(png->data.begin(), png->data.end());

    std::ofstream out(outPath, std::ios::binary | std::ios::trunc);
    if (!out)
    {
        std::cout << "Failed to open file: " << outPath << std::endl;
        return;
    }

    out.write(reinterpret_cast<char*>(&texture.header), sizeof(TextureHeader));
    out.write(reinterpret_cast<char*>(texture.data.data()), texture.data.size());

    if (!out)
    {
        std::cout << "Write failed for " << outPath.string() << "\n";
    }
    else
    {
        std::cout << "Saved texture.bin: " << outPath.string() << "\n";
    }
}

void SaveTextureArray(const std::filesystem::path& inPath, const std::filesystem::path& outPath)
{
    std::vector<Png> pngs;
    int width = -1, height = -1;
    for (const auto& file : std::filesystem::directory_iterator(inPath))
    {
        if (!file.is_regular_file()) continue;
        std::optional<Png> png = ReadRgbaPng(file);
        if (png)
        {
            // First image sets width and height, the rest must have the same
            if (width == -1 || height == -1)
            {
                width = png->width;
                height = png->height;
            }

            if (png->width == width && png->height == height)
            {
                pngs.push_back(*png);
            }
            else
            {
                std::cout << "Skipped '" << file.path().string() << "' while creating texture array because of its dimensions" << std::endl;
            }
        }
    }
    // No pngs...
    if (width == -1 || height == 1)
    {
        std::cout << "Failed to save texture array: " << inPath.string() << std::endl;
        return;
    }

    Texture texture;
    texture.header.magic = MakeMagic('T', 'E', 'X', 'A');
    texture.header.version = 1;
    texture.header.width = width;
    texture.header.height = height;
    texture.header.channels = 4;
    texture.header.layers = pngs.size();
    texture.header.dataSize = width * height * 4 * pngs.size();
    for (const auto& png : pngs)
    {
        texture.data.insert(texture.data.end(), png.data.begin(), png.data.end());
    }

    std::ofstream out(outPath, std::ios::binary | std::ios::trunc);
    if (!out)
    {
        std::cout << "Failed to open file: " << outPath << std::endl;
        return;
    }

    out.write(reinterpret_cast<char*>(&texture.header), sizeof(TextureHeader));
    out.write(reinterpret_cast<char*>(texture.data.data()), texture.data.size());

    if (!out)
    {
        std::cout << "Write failed for " << outPath.string() << "\n";
    }
    else
    {
        std::cout << "Saved texture_array.bin: " << outPath.string() << "\n";
    }
}