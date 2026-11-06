#pragma once

#ifndef NETCOMP_PNG_H
#define NETCOMP_PNG_H

#include <cstdint>
#include <filesystem>
#include <vector>

struct Png
{
    int width, height, channels;
    std::vector<uint8_t> data;
};

Png ReadRgbaPng(const std::filesystem::path& path);
Png ReadAlphaPng(const std::filesystem::path& path);

#endif //NETCOMP_PNG_H