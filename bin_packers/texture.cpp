#include "bin_packers/texture.h"

#include <fstream>
#include <iostream>

#include "raw_assets/png.h"
#include "util/magic.h"

void SaveTexture(const std::filesystem::path& inPath, const std::filesystem::path& outPath, const bool rgba)
{
    std::optional<Png> png = rgba ? ReadRgbaPng(inPath) : ReadAlphaPng(inPath);
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

std::optional<TextureKeys> SaveTextureArray(const std::filesystem::path& inPath, const std::filesystem::path& outPath)
{
    // Sorting the files
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(inPath))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".png")
            files.push_back(entry.path());
    }
    std::ranges::sort(files);

    std::vector<Png> pngs;
    TextureKeys keys;
    int width = -1, height = -1;

    for (const auto& path : files)
    {
        std::optional<Png> png = ReadRgbaPng(path);
        if (!png) continue;

        // First image sets width and height, the rest must match
        if (width == -1 || height == -1)
        {
            width = png->width;
            height = png->height;
        }
        if (png->width != width || png->height != height)
        {
            std::cout << "Skipped '" << path.string() << "' while creating texture array because of its dimensions" << std::endl;
            continue;
        }

        // The layer index is the position it's about to take in the vector
        keys[path.filename().string()] = static_cast<uint32_t>(pngs.size());
        pngs.push_back(std::move(*png));
    }

    // No pngs...
    if (pngs.empty())
    {
        std::cout << "Failed to save texture array: " << inPath.string() << std::endl;
        return std::nullopt;
    }

    Texture texture;
    texture.header.magic = MakeMagic('T', 'E', 'X', 'A');
    texture.header.version = 1;
    texture.header.width = width;
    texture.header.height = height;
    texture.header.channels = 4;
    texture.header.layers = static_cast<uint32_t>(pngs.size());
    texture.header.dataSize = width * height * 4 * pngs.size();

    texture.data.reserve(texture.header.dataSize);
    for (const auto& png : pngs)
        texture.data.insert(texture.data.end(), png.data.begin(), png.data.end());

    std::ofstream out(outPath, std::ios::binary | std::ios::trunc);
    if (!out)
    {
        std::cout << "Failed to open file: " << outPath << std::endl;
        return std::nullopt;
    }

    out.write(reinterpret_cast<const char*>(&texture.header), sizeof(TextureHeader));
    out.write(reinterpret_cast<const char*>(texture.data.data()), static_cast<std::streamsize>(texture.data.size()));

    if (!out)
    {
        std::cout << "Write failed for " << outPath.string() << "\n";
        return std::nullopt;
    }

    std::cout << "Saved texture array: " << outPath.string() << " (" << pngs.size() << " layers)\n";
    return keys;
}