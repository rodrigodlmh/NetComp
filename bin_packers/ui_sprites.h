#pragma once

#ifndef NETCOMP_UI_SPRITES_H
#define NETCOMP_UI_SPRITES_H

#include <filesystem>
#include <vector>

struct UiHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t spriteCount;
};

struct UiSprite
{
    float uvMinX;
    float uvMinY;
    float uvMaxX;
    float uvMaxY;
};

struct UiSprites
{
    UiHeader header;
    std::vector<UiSprite> sprites;
};

void SaveUiSprites(const std::filesystem::path& inPath, const std::filesystem::path& outPath);

#endif //NETCOMP_UI_SPRITES_H