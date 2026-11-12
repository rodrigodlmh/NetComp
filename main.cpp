#include <filesystem>

#include "bin_packers/lang.h"
#include "bin_packers/font.h"

int main()
{
    const std::filesystem::path inPath = R"(C:\Projects\Build\in)";
    const std::filesystem::path outPath = R"(C:\Projects\Build\out)";

    SaveLanguages(inPath / "lang" / "lang.tsv", outPath / "lang");
    SaveFont(inPath / "ui" / "font.json", outPath / "ui");

   return 0;
}