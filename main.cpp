#include "gb_packers/lang.h"
#include "raw_assets/tsv.h"
#include "util/diagnostics.h"
#include "util/vfs.h"

int main()
{
    const std::filesystem::path assetPath = R"(C:\Projects\Build\in)";
    const std::filesystem::path outPath = R"(C:\Projects\Build\out)";
    std::filesystem::create_directory(outPath / "lang");
    std::filesystem::create_directory(outPath / "shaders");
    std::filesystem::create_directory(outPath / "textures");

    EnableConsoleColors();
    DiagnosticSink diagnostics;
    diagnostics.SetListener(PrintDiagnostic);

    std::vector<std::filesystem::path> packs;
    for (const auto& pack : std::filesystem::directory_iterator(assetPath))
        if (pack.is_directory()) packs.push_back(pack);
    std::ranges::sort(packs, [](const std::filesystem::path& a, const std::filesystem::path& b) {
        bool ab = a.filename() == "netlink", bb = b.filename() == "netlink";
        if (ab != bb) return ab;
        return a.filename() < b.filename();
    });

    Vfs vfs;
    for (const auto& pack : packs)
    {
        vfs.AddPack(pack, diagnostics);
    }

    std::vector<VfsEntry> tsvsEntries = vfs.List(FileType::TSV);
    std::vector<Tsv> tsvs(tsvsEntries.size());
    for (size_t i = 0; i < tsvsEntries.size(); ++i)
    {
        tsvs[i] = ReadTsv(tsvsEntries[i].path);
    }
    LangKeys keys = GetLangKeys(tsvs, diagnostics);
    LangNames names = GetLangNames(tsvs, diagnostics);
    for (const auto& lang : names.names)
    {
        LangData data = GetLangData(tsvs, keys, lang, diagnostics);
        SaveLang(data, outPath / "lang", diagnostics);
    }


    return 0;
}