#include "bin_packers/lang.h"

#include <fstream>
#include <iostream>

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

LangStringMetrics MeasureString(const uint8_t* strData, const FontGlyphs& glyphs)
{
    LangStringMetrics m{};
    uint32_t index = 0;
    for (;;)
    {
        const Utf8Result r = Next(strData, index);
        if (r.codepoint == '\0') break;

        index = r.nextIndex;

        const FontGlyph glyph = GetGlyph(glyphs, r.codepoint);
        m.width += glyph.advance;
        m.glyphCount++;
    }
    return m;
}

std::optional<LangKeys> SaveLanguages(const std::filesystem::path& inPath, const FontGlyphs& glyphs, const std::filesystem::path& outPath)
{
    const std::optional<Tsv> tsv = ReadTsv(inPath);
    if (!tsv)
    {
        std::cout << "Failed to read TSV file: " << inPath.string() << std::endl;
        return std::nullopt;
    }
    if (tsv->headers.size() < 2)
    {
        std::cout << "TSV needs a key column and at least one language column: " << inPath.string() << std::endl;
        return std::nullopt;
    }

    const size_t langCount = tsv->headers.size() - 1; // Minus the key column
    const size_t rowCount = tsv->rows.size();

    LangKeys keys;
    keys.reserve(rowCount);
    for (size_t ri = 0; ri < rowCount; ++ri)
    {
        const auto& row = tsv->rows[ri];
        if (row.empty() || row[0].empty())
        {
            std::cout << "Empty key at row " << ri << "\n";
            return std::nullopt;
        }

        const auto [it, inserted] = keys.emplace(row[0], static_cast<uint32_t>(ri));
        if (!inserted)
        {
            std::cout << "Duplicate key '" << row[0] << "' at rows " << it->second << " and " << ri << "\n";
            return std::nullopt;
        }
    }

    std::vector<Lang> langs(langCount);

    uint32_t maxBlobSize = 0;
    for (size_t li = 0; li < langCount; ++li)
    {
        const size_t col = li + 1; // Skip the key column
        const std::string& langName = tsv->headers[col];

        Lang& lang = langs[li];
        lang.strings.reserve(rowCount);

        for (size_t ri = 0; ri < rowCount; ++ri)
        {
            const auto& row = tsv->rows[ri];
            const std::string& key = row[0]; // Validated above

            const bool missing = col >= row.size() || row[col].empty();
            if (missing) std::cout << "[" << langName << "] Missing translation for '" << key << "'\n";
            const std::string text = missing ? "?" : row[col];

            // The blob offset (and final blob size) must fit in a uint32_t. +1 for the NUL terminator.
            if (text.size() + 1 > UINT32_MAX - lang.blob.size())
            {
                std::cout << "[" << langName << "] Blob exceeds 4 GB, key '" << key << "'\n";
                return std::nullopt;
            }

            const LangStringMetrics m = MeasureString(reinterpret_cast<const uint8_t*>(text.c_str()), glyphs);

            LangString s{};
            s.offset     = static_cast<uint32_t>(lang.blob.size());
            s.byteCount  = static_cast<uint32_t>(text.size());
            s.glyphCount = m.glyphCount;
            s.width      = m.width;

            lang.strings.push_back(s);
            lang.blob.insert(lang.blob.end(), text.begin(), text.end());
            lang.blob.push_back('\0');
        }

        lang.header.magic       = MakeMagic('L', 'A', 'N', 'G');
        lang.header.version     = 2; // LangString layout changed
        lang.header.langCount   = static_cast<uint32_t>(langCount);
        lang.header.stringCount = static_cast<uint32_t>(lang.strings.size());
        lang.header.blobSize    = static_cast<uint32_t>(lang.blob.size());

        maxBlobSize = std::max(maxBlobSize, lang.header.blobSize);
    }

    // Set same max blob size for all
    for (Lang& lang : langs) lang.header.maxBlobSize = maxBlobSize;

    std::error_code ec;
    std::filesystem::create_directories(outPath, ec);
    if (ec)
    {
        std::cout << "Failed to create " << outPath.string() << ": " << ec.message() << "\n";
        return std::nullopt;
    }
    for (size_t li = 0; li < langCount; ++li)
        if (!WriteLang(outPath / (tsv->headers[li + 1] + ".bin"), langs[li]))
            return std::nullopt;

    return keys;
}

bool WriteLang(const std::filesystem::path& path, const Lang& lang)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
    {
        std::cout << "Failed to open file while writing lang: " << path.string() << std::endl;
        return false;
    }

    file.write(reinterpret_cast<const char*>(&lang.header), sizeof(LangHeader));
    file.write(reinterpret_cast<const char*>(lang.strings.data()), static_cast<std::streamsize>(lang.strings.size() * sizeof(LangString)));
    file.write(reinterpret_cast<const char*>(lang.blob.data()), static_cast<std::streamsize>(lang.blob.size()));

    if (!file)
    {
        std::cout << "Failed to save file while writing lang: " << path.string() << std::endl;
        return false;
    }
    std::cout << "Saved lang.bin: " << path.string() << std::endl;
    return true;
}