#pragma once

#ifndef NETCOMP_UI_H
#define NETCOMP_UI_H

#include <filesystem>
#include <vector>

struct UiHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t spriteCount;
    uint32_t atlasWidth;
    uint32_t atlasHeight;
    uint32_t atlasChannels;
    uint32_t atlasDataSize;
};

struct UiSprite
{
    float uvMinX;
    float uvMinY;
    float uvMaxX;
    float uvMaxY;
};

struct Ui
{
    UiHeader header;
    std::vector<UiSprite> sprites;
    std::vector<uint8_t> atlasBlob;
};

void SaveUi(const std::filesystem::path& inPath, const std::filesystem::path& outPath);

#endif //NETCOMP_UI_H