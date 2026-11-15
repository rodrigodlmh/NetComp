#pragma once

#ifndef NETCOMP_TEXTURE_H
#define NETCOMP_TEXTURE_H

#include <cstdint>
#include <vector>
#include <filesystem>

struct TextureHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    uint32_t layers;
    uint32_t dataSize;
};

struct Texture
{
    TextureHeader header;
    std::vector<uint8_t> data;
};

// inPath must be a path to a file
void SaveTexture(const std::filesystem::path& inPath, const std::filesystem::path& outPath);
// inPath must be a path to a folder with png files
void SaveTextureArray(const std::filesystem::path& inPath, const std::filesystem::path& outPath);

#endif //NETCOMP_TEXTURE_H