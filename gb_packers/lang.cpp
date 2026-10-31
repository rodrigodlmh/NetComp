#include "gb_packers/lang.h"

#include <unordered_set>

#include "util/hash.h"

bool CheckTsvCorrectness(const RawTsv& tsv, Diagnostics& diagnostics)
{
    bool valid = true;
    if (tsv.source.empty())
    {
        diagnostics.Error(tsv.source, "Empty tsv file");
        valid = false;
    }
    if (tsv.headers.empty())
    {
        diagnostics.Error(tsv.source, "Missing headers");
        valid = false;
    }
    if (tsv.headers[0] != "key")
    {
        diagnostics.Error(tsv.source, "Missing key column (must be the first column)");
        valid = false;
    }
    if (tsv.rows.empty())
    {
        diagnostics.Error(tsv.source, "Tsv file has no rows");
        valid = false;
    }
    return valid;
}

LangKeys GetLangKeys(const std::vector<RawTsv>& tsvFiles, Diagnostics& diagnostics)
{
    LangKeys keys;

    std::unordered_map<uint64_t, std::string> hashMap;
    for (const auto & tsv : tsvFiles)
    {
        if (!CheckTsvCorrectness(tsv, diagnostics)) continue;

        for (const auto & row : tsv.rows)
        {
            if (row.cells.empty() || row.cells[0].empty())
            {
                diagnostics.Error(tsv.source, "Missing row", row.line);
                continue;
            }
            const auto& key = row.cells[0];
            uint64_t hash = HashStr(key);
            auto [it, inserted] = hashMap.emplace(hash, key);

            if (!inserted)
            {
                if (it->second != key)
                {
                    diagnostics.Error(tsv.source, "Hash collision, try renaming the key", row.line);
                }
                else
                {
                    diagnostics.Error(tsv.source, "Duplicate key", row.line);
                }

                continue;
            }
            const auto index = static_cast<uint32_t>(keys.keys.size());
            keys.keys.push_back(key);
            keys.keyHashes.push_back(hash);
            keys.keyToIndex.emplace(hash, index);
        }
    }
    return keys;
}

LangNames GetLangNames(const std::vector<RawTsv>& tsvFiles, Diagnostics& diagnostics)
{
    LangNames names;

    std::unordered_set<std::string> allLanguages;
    for (const auto & tsv : tsvFiles)
    {
        if (!CheckTsvCorrectness(tsv, diagnostics)) continue;

        // Get languages
        std::unordered_set<std::string> fileLanguages;
        for (size_t i = 1; i < tsv.headers.size(); ++i)
        {
            const auto& header = tsv.headers[i];

            if (header.empty())
            {
                diagnostics.Error(tsv.source, "Empty language header");
                continue;
            }

            if (!fileLanguages.emplace(header).second)
            {
                diagnostics.Error(tsv.source, "Duplicate language header: " + header);
                continue;
            }

            if (allLanguages.emplace(header).second)
            {
                names.names.push_back(header);
            }
        }
    }
    return names;
}

LangData GetLang(const std::vector<RawTsv>& tsvFiles, const LangKeys& keys, const std::string& langName, Diagnostics& diagnostics)
{
    LangData lang;
    lang.name = langName;
    lang.entries.resize(keys.keys.size());

    for (const auto& tsv : tsvFiles)
    {
        if (!CheckTsvCorrectness(tsv, diagnostics)) continue;

        // Find language column
        size_t langIndex = 0;
        for (size_t i = 0; i < tsv.headers.size(); ++i)
        {
            if (tsv.headers[i] == langName) langIndex = i;
            break;
        }

        // This TSV doesn't contain this language.
        if (langIndex == tsv.headers.size()) continue;


        for (const auto& row : tsv.rows)
        {
            if (row.cells.empty() || row.cells[0].empty()) continue;
            lang.entries.push_back(row.cells[langIndex]);

            const auto& key = row.cells[0];
            const uint64_t hash = HashStr(key);

            auto it = keys.keyToIndex.find(hash);

            if (it == keys.keyToIndex.end()) continue;

            const uint32_t index = it->second;
            if (langIndex >= row.cells.size())
            {
                diagnostics.Error(tsv.source, "Missing translation column", row.line);
                continue;
            }
            lang.entries[index] = row.cells[langIndex];
        }
    }

    return lang;
}