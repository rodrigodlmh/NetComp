#pragma once

#ifndef NETCOMP_LANG_H
#define NETCOMP_LANG_H

#include <cstdint>
#include <vector>
#include <filesystem>

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

struct LangString
{
    uint32_t offset;
    uint16_t glyphCount;
    uint16_t byteCount;
};

struct Lang
{
    LangHeader header;
    std::vector<LangString> strings;
    std::vector<uint8_t> blob;
};

inline bool IsContinuation(const uint8_t b) { return (b & 0xC0) == 0x80; }
// NUL terminated strings
[[nodiscard]] Utf8Result Next(const uint8_t* str, const uint32_t index);
[[nodiscard]] uint32_t Count(const uint8_t* str);

void SaveLanguages(const std::filesystem::path& inPath, const std::filesystem::path& outPath);
void WriteLang(const std::filesystem::path& path, const Lang& lang);

#endif //NETCOMP_LANG_H