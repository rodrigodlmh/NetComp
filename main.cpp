#include "gb_packers/font.h"
#include "gb_packers/settings.h"
#include "gb_packers/lang.h"
#include "raw_assets/tsv.h"
#include "util/diagnostics.h"
#include "util/vfs.h"

int main()
{
    const std::filesystem::path assetPath = R"(C:\Projects\Build\in)";
    const std::filesystem::path outPath = R"(C:\Projects\Build\out)";
    std::filesystem::create_directory(outPath / "lang");
    std::filesystem::create_directory(outPath / "ui");
    std::filesystem::create_directory(outPath / "shaders");
    std::filesystem::create_directory(outPath / "textures");

    EnableConsoleColors();
    DiagnosticSink diagnostics;
    diagnostics.SetListener(PrintDiagnostic);

    // Compile assets from global settings
    std::optional<GlobalSettings> settings = GetGlobalSettings(assetPath / "global_settings.json", diagnostics);
    if (!settings.has_value())
    {
        diagnostics.Error("Couldn't find 'global_settings.json'", assetPath);
        return 0;
    }

    Vfs vfs;
    Definitions defs;

    for (const auto& pack : settings.value().packs)
    {
        const auto packPath = assetPath / pack;
        vfs.AddPack(packPath, pack, diagnostics);

        if (auto local = GetLocalSettings(packPath / "pack.json", diagnostics))
            OverrideDefinitions(defs, local);
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

    std::optional<std::string> fontData = GetDef(defs, Defs::FontData);
    if (fontData.has_value())
    {
        std::optional<std::filesystem::path> fontAsset = vfs.GetAsset(fontData.value());
        if (!fontAsset.has_value())
        {
            diagnostics.Error("Failed to find font asset referenced in global settings");
            return 0;
        }
        SaveFont(vfs, fontAsset.value(), outPath / "ui" / "font.pak", diagnostics);
    }

    return 0;
}