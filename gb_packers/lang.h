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

bool CheckTsvCorrectness(const RawTsv& tsv, Diagnostics& diagnostics);
LangKeys GetLangKeys(const std::vector<RawTsv>& tsvFiles, Diagnostics& diagnostics);
LangNames GetLangNames(const std::vector<RawTsv>& tsvFiles, Diagnostics& diagnostics);
LangData GetLang(const std::vector<RawTsv>& tsvFiles, const LangKeys& keys, const std::string& langName, Diagnostics& diagnostics);

#endif //NETCOMP_LANG_H