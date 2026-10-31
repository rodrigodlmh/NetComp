#include <iostream>

#include "gb_packers/lang.h"

int main()
{
    const std::filesystem::path currentPath = std::filesystem::current_path();
    const std::filesystem::path path = currentPath.parent_path() / "test_assets";
    std::cout << path << std::endl;

    std::vector<RawTsv> tsv;
    for (const auto& file : std::filesystem::directory_iterator(path))
    {
        if (file.path().extension() == ".tsv")
        {
            std::cout << file.path() << std::endl;
            tsv.emplace_back(ReadTsv(file.path()));
            break;
        }
    }

    Diagnostics diagnostics;
    LangKeys keys = GetLangKeys(tsv, diagnostics);
    LangNames languages = GetLangNames(tsv, diagnostics);

    std::filesystem::path outPath = path / "out";
    for (const auto& lang : languages.names)
    {
        LangData data = GetLang(tsv, keys, lang, diagnostics);
        SaveLang(data, outPath, diagnostics);
    }
    for (const auto& error : diagnostics.errors)
    {
        std::cout << error << std::endl;
    }
    return 0;
}