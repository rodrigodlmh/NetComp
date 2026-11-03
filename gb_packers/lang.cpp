#include "gb_packers/lang.h"

#include <unordered_set>
#include <fstream>

#include "utf8.h"

#include "util/hash.h"
#include "util/magic.h"

bool CheckTsvCorrectness(const Tsv& tsv, DiagnosticSink& diagnostics)
{
    bool valid = true;
    if (tsv.source.empty())
    {
        diagnostics.Error("Empty tsv file", tsv.source);
        valid = false;
    }
    if (tsv.headers.empty())
    {
        diagnostics.Error("Missing headers in tsv file" , tsv.source);
        return false; // To avoid reading headers[0] later
    }
    if (tsv.headers[0] != "key")
    {
        diagnostics.Error("Missing key column (must be the first column)", tsv.source);
        valid = false;
    }
    if (tsv.rows.empty())
    {
        diagnostics.Error("Tsv file has no rows" , tsv.source);
        valid = false;
    }
    return valid;
}

LangKeys GetLangKeys(const std::vector<Tsv>& tsvFiles, DiagnosticSink& diagnostics)
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
                diagnostics.Error("Missing row", tsv.source, row.line);
                continue;
            }
            const auto& key = row.cells[0];
            uint64_t hash = HashStr(key);
            auto [it, inserted] = hashMap.emplace(hash, key);

            if (!inserted)
            {
                if (it->second != key)
                {
                    diagnostics.Error("Hash collision, try renaming the key", tsv.source, row.line);
                }
                else
                {
                    diagnostics.Error("Duplicate key", tsv.source, row.line);
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

LangNames GetLangNames(const std::vector<Tsv>& tsvFiles, DiagnosticSink& diagnostics)
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
                diagnostics.Error("Empty language header", tsv.source);
                continue;
            }

            if (!fileLanguages.emplace(header).second)
            {
                diagnostics.Error("Duplicate language header: " + header, tsv.source);
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

LangData GetLang(const std::vector<Tsv>& tsvFiles, const LangKeys& keys, const std::string& langName, DiagnosticSink& diagnostics)
{
    LangData lang;
    lang.name = langName;
    lang.entries.resize(keys.keys.size());

    for (const auto& tsv : tsvFiles)
    {
        if (!CheckTsvCorrectness(tsv, diagnostics)) continue;

        // Find language column
        size_t langIndex = tsv.headers.size();
        for (size_t i = 0; i < tsv.headers.size(); ++i)
        {
            if (tsv.headers[i] == langName)
            {
                langIndex = i;
                break;
            }
        }

        // This TSV doesn't contain this language.
        if (langIndex == tsv.headers.size()) continue;
        
        for (const auto& row : tsv.rows)
        {
            if (row.cells.empty() || row.cells[0].empty()) continue;

            const auto& key = row.cells[0];
            const uint64_t hash = HashStr(key);

            auto it = keys.keyToIndex.find(hash);

            if (it == keys.keyToIndex.end()) continue;

            const uint32_t index = it->second;
            if (langIndex >= row.cells.size())
            {
                diagnostics.Error("Missing translation column", tsv.source, row.line);
                continue;
            }
            lang.entries[index] = row.cells[langIndex];
        }
    }

    return lang;
}

void SaveLang(const LangData& data, const std::filesystem::path& path, DiagnosticSink& diagnostics)
{
    if (!std::filesystem::is_directory(path))
    {
        diagnostics.Error("Provided path for saving is not a directory", path);
        return;
    }

    const auto savePath = path / ("lang_" + data.name + ".pak");

    constexpr size_t maxStringSize = std::numeric_limits<uint16_t>::max();

    std::vector<uint8_t> blob;
    std::vector<LangPakString> strings;
    strings.reserve(data.entries.size());

    for (const auto& string : data.entries)
    {
        if (string.size() > maxStringSize)
        {
            diagnostics.Error("Translation " + string +  " exceeds maximum size of 65535 bytes");

            const auto offset = static_cast<uint32_t>(blob.size());
            strings.push_back({ offset, 0, 0 });
            blob.push_back('\0');
            continue;
        }

        const size_t codepointCount = utf8::distance(string.begin(), string.end());

        if (codepointCount > std::numeric_limits<uint16_t>::max())
        {
            diagnostics.Error("Translation " + string +  " exceeds maximum codepoint count");

            const auto offset = static_cast<uint32_t>(blob.size());
            strings.push_back({ offset, 0, 0 });
            blob.push_back('\0');
            continue;
        }

        const auto offset = static_cast<uint32_t>(blob.size());
        const auto byteCount = static_cast<uint16_t>(string.size());
        const auto glyphCount = static_cast<uint16_t>(codepointCount);

        strings.push_back({ offset, glyphCount, byteCount });

        blob.insert(blob.end(), string.begin(), string.end());
        blob.push_back('\0');
    }

    if (strings.empty() || blob.empty())
    {
        diagnostics.Error("Skipped empty language: " + data.name);
    }

    LangPakHeader header{};
    header.magic = MakeMagic('L', 'P', 'A', 'K');
    header.version = 1;
    header.stringCount = static_cast<uint32_t>(strings.size());
    header.blobSize = static_cast<uint32_t>(blob.size());

    std::ofstream outFile(savePath, std::ios::binary);
    if (!outFile)
    {
        diagnostics.Error("Failed to open file for writing", savePath);
        return;
    }
    outFile.write(reinterpret_cast<const char*>(&header), sizeof(LangPakHeader));
    outFile.write(reinterpret_cast<const char*>(strings.data()), static_cast<std::streamsize>(strings.size() * sizeof(LangPakString)));
    outFile.write(reinterpret_cast<const char*>(blob.data()), static_cast<std::streamsize>(blob.size()));
    if (!outFile)
    {
        diagnostics.Error("Failed to write language file", savePath);
    }
}