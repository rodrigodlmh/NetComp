#pragma once

#ifndef NETCOMP_PNG_H
#define NETCOMP_PNG_H

#include <cstdint>
#include <filesystem>
#include <vector>
#include <optional>

struct Png
{
    int width, height, channels;
    std::vector<uint8_t> data;
};

std::optional<Png> ReadRgbaPng(const std::filesystem::path& path);
std::optional<Png> ReadAlphaPng(const std::filesystem::path& path);

#endif //NETCOMP_PNG_H