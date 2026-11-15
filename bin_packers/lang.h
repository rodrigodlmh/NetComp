#pragma once

#ifndef NETCOMP_LANG_H
#define NETCOMP_LANG_H

#include <cstdint>
#include <vector>
#include <filesystem>

#include "font_glyphs.h"

struct Utf8Result
{
    uint32_t codepoint;
    uint32_t nextIndex;
};

struct LangHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t langCount;
    uint32_t stringCount;
    uint32_t maxBlobSize;
    uint32_t blobSize;
};

struct LangStringMetrics
{
    uint32_t glyphCount; uint32_t width;
};

struct LangString
{
    uint32_t offset;
    uint32_t glyphCount;
    uint32_t byteCount;
    uint32_t width;
};

struct Lang
{
    LangHeader header;
    std::vector<LangString> strings;
    std::vector<uint8_t> blob;
};

using LangKeys = std::unordered_map<std::string, uint32_t>;

inline bool IsContinuation(const uint8_t b) { return (b & 0xC0) == 0x80; }
// NUL terminated strings
[[nodiscard]] Utf8Result Next(const uint8_t* str, uint32_t index);
[[nodiscard]] uint32_t Count(const uint8_t* str);
[[nodiscard]] LangStringMetrics MeasureString(const uint8_t* strData, const FontGlyphs& glyphs);

std::optional<LangKeys> SaveLanguages(const std::filesystem::path& inPath, const FontGlyphs& glyphs, const std::filesystem::path& outPath);
bool WriteLang(const std::filesystem::path& path, const Lang& lang);

#endif //NETCOMP_LANG_H