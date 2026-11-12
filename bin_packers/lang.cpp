#include "bin_packers/lang.h"

#include <fstream>
#include <iostream>

#include "utf8.h"
#include "raw_assets/tsv.h"
#include "util/magic.h"

Utf8Result Next(const uint8_t* str, const uint32_t index)
{
    const uint8_t c = str[index];

    // End of string check
    if (c == '\0') return { 0, index };

    // 1-byte (ASCII: 01-7F)
    if (c <= 0x7F) return { c, index + 1 };

    // 2-byte (C2-DF)
    if (c >= 0xC2 && c <= 0xDF)
    {
        const uint8_t b1 = str[index + 1];
        if (!IsContinuation(b1)) return { 0xFFFD, index + 1 };

        const uint32_t cp = ((c & 0x1F) << 6) | (b1 & 0x3F);
        return { cp, index + 2 };
    }

    // 3-byte (E0-EF)
    if (c >= 0xE0 && c <= 0xEF)
    {
        const uint8_t b1 = str[index + 1];
        if (!IsContinuation(b1)) return { 0xFFFD, index + 1 };

        // Prevent overlong encodings and UTF-16 surrogates.
        if (c == 0xE0 && b1 < 0xA0) return { 0xFFFD, index + 1 };
        if (c == 0xED && b1 > 0x9F) return { 0xFFFD, index + 1 };

        // SAFE: We verified b1 is a valid continuation, so it's not '\0'.
        // It is now perfectly safe to read index + 2.
        const uint8_t b2 = str[index + 2];
        if (!IsContinuation(b2)) return { 0xFFFD, index + 2 };

        const uint32_t cp = ((c & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
        return { cp, index + 3 };
    }

    // 4-byte (F0-F4)
    if (c >= 0xF0 && c <= 0xF4)
    {
        const uint8_t b1 = str[index + 1];
        if (!IsContinuation(b1)) return { 0xFFFD, index + 1 };

        // Prevent overlong encodings and code points above U+10FFFF.
        if (c == 0xF0 && b1 < 0x90) return { 0xFFFD, index + 1 };
        if (c == 0xF4 && b1 > 0x8F) return { 0xFFFD, index + 1 };

        // SAFE: b1 is verified, safe to read index + 2
        const uint8_t b2 = str[index + 2];
        if (!IsContinuation(b2)) return { 0xFFFD, index + 2 };

        // SAFE: b2 is verified, safe to read index + 3
        const uint8_t b3 = str[index + 3];
        if (!IsContinuation(b3)) return { 0xFFFD, index + 3 };

        const uint32_t cp = ((c & 0x07) << 18) | ((b1 & 0x3F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
        return { cp, index + 4 };
    }

    // Invalid leading byte
    return { 0xFFFD, index + 1 };
}

uint32_t Count(const uint8_t* str)
{
    uint32_t count = 0;
    uint32_t index = 0;
    while (true)
    {
        const Utf8Result r = Next(str, index);
        if (r.codepoint == '\0') break;
        index = r.nextIndex;
        count++;
    }
    return count;
}

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
            s.glyphCount = static_cast<uint16_t>(Count(reinterpret_cast<const uint8_t*>(text.c_str())));

            lang.strings.push_back(s);
            lang.blob.insert(lang.blob.end(), text.begin(), text.end());
            lang.blob.push_back('\0');
        }

        lang.header.magic       = MakeMagic('L', 'A', 'N', 'G');
        lang.header.version     = 1;
        lang.header.langCount   = static_cast<uint32_t>(langCount);
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