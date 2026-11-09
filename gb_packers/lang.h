#pragma once

#ifndef NETCOMP_LANG_H
#define NETCOMP_LANG_H

#include <unordered_map>
#include <vector>

#include "raw_assets/tsv.h"
#include "util/diagnostics.h"

struct LangKeys
{
    std::vector<std::string> keys;
    std::vector<uint64_t> keyHashes;
    std::unordered_map<uint64_t, uint32_t> keyToIndex;
};

struct LangNames
{
    std::vector<std::string> names;
};

struct LangData
{
    std::string name;
    std::vector<std::string> entries;
};

struct LangPakHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t maxStringCount;
    uint32_t stringCount;
    uint32_t maxBlobSize;
    uint32_t blobSize;
};

struct LangPakString
{
    uint32_t offset;
    uint16_t glyphCount;
    uint16_t byteCount;
};

struct LangPak
{
    LangPakHeader header;
    std::vector<LangPakString> strings;
    std::vector<uint8_t> blob;
};

bool CheckTsvCorrectness(const Tsv& tsv, DiagnosticSink& diagnostics);
LangKeys GetLangKeys(const std::vector<Tsv>& tsvFiles, DiagnosticSink& diagnostics);
LangNames GetLangNames(const std::vector<Tsv>& tsvFiles, DiagnosticSink& diagnostics);
LangData GetLangData(const std::vector<Tsv>& tsvFiles, const LangKeys& keys, const std::string& langName, DiagnosticSink& diagnostics);
void SaveLang(const LangData& data, const std::filesystem::path& path, DiagnosticSink& diagnostics);

#endif //NETCOMP_LANG_H