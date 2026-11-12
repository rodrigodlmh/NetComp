#include "bin_packers/lang.h"

#include <fstream>
#include <iostream>

#include "utf8.h"
#include "raw_assets/tsv.h"
#include "util/magic.h"

void SaveLanguages(const std::filesystem::path& inPath, const std::filesystem::path& outPath)
{
    const std::optional<Tsv> tsv = ReadTsv(inPath);
    if (!tsv)
    {
        std::cout << "Failed to read TSV file: " << inPath.string() << std::endl;
        return;
    }
    if (tsv->headers.size() < 2)
    {
        std::cout << "TSV needs a key column and at least one language column: " << inPath.string() << std::endl;
        return;
    }

    const size_t langCount = tsv->headers.size() - 1; // Minus the key column
    const size_t rowCount = tsv->rows.size();

    std::vector<Lang> langs(langCount);

    uint32_t maxBlobSize = 0;
    for (size_t li = 0; li < langCount; ++li)
    {
        const size_t col = li + 1; // Skip the key column
        const std::string& langName = tsv->headers[col];

        Lang& lang = langs[li];
        lang.strings.reserve(rowCount);

        for (const auto& row : tsv->rows)
        {
            const std::string key  = (!row.empty() && !row[0].empty()) ? row[0] : "?";
            const std::string text = (col < row.size() && !row[col].empty()) ? row[col] : "?";

            if (!utf8::is_valid(text.begin(), text.end()))
            {
                std::cout << "[" << langName << "] Invalid UTF-8, key '" << key << "'" << std::endl;
                return;
            }

            if (text.size() > UINT16_MAX)
            {
                std::cout << "[" << langName << "] String too long (>65535 bytes), key '" << key << "'" << std::endl;
                return;
            }

            if (lang.blob.size() > UINT32_MAX - text.size())
            {
                std::cout << "[" << langName << "] Blob exceeds 4 GB, key '" << key << "'" << std::endl;
                return;
            }

            const size_t glyphs = static_cast<size_t>(utf8::distance(text.begin(), text.end()));

            LangString s{};
            s.offset     = static_cast<uint32_t>(lang.blob.size());
            s.byteCount  = static_cast<uint16_t>(text.size());
            s.glyphCount = static_cast<uint16_t>(glyphs);

            lang.strings.push_back(s);
            lang.blob.insert(lang.blob.end(), text.begin(), text.end());
        }

        lang.header.magic       = MakeMagic('L', 'A', 'N', 'G');
        lang.header.version     = 1;
        lang.header.stringCount = static_cast<uint32_t>(lang.strings.size());
        lang.header.blobSize    = static_cast<uint32_t>(lang.blob.size());

        maxBlobSize = std::max(maxBlobSize, lang.header.blobSize);
    }

    // Set same max blob size for all
    for (Lang& lang : langs) lang.header.maxBlobSize = maxBlobSize;

    std::filesystem::create_directories(outPath);
    for (size_t li = 0; li < langCount; ++li)
        WriteLang(outPath / (tsv->headers[li + 1] + ".bin"), langs[li]);
}

void WriteLang(const std::filesystem::path& path, const Lang& lang)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
    {
        std::cout << "Failed to open file while writing lang: " << path.string() << std::endl;
        return;
    }

    file.write(reinterpret_cast<const char*>(&lang.header), sizeof(LangHeader));
    file.write(reinterpret_cast<const char*>(lang.strings.data()), static_cast<std::streamsize>(lang.strings.size() * sizeof(LangString)));
    file.write(reinterpret_cast<const char*>(lang.blob.data()), static_cast<std::streamsize>(lang.blob.size()));

    if (!file)
    {
        std::cout << "Failed to save file while writing lang: " << path.string() << std::endl;
    }
    else
    {
        std::cout << "Saved lang.bin: " << path.string() << std::endl;
    }
}