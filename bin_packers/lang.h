#pragma once

#ifndef NETCOMP_LANG_H
#define NETCOMP_LANG_H

#include <cstdint>
#include <vector>
#include <filesystem>

struct LangHeader
{
    uint32_t magic;
    uint32_t version;
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

void SaveLanguages(const std::filesystem::path& inPath, const std::filesystem::path& outPath);
void WriteLang(const std::filesystem::path& path, const Lang& lang);

#endif //NETCOMP_LANG_H