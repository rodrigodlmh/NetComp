#include "util/diagnostics.h"
#include "util/vfs.h"

int main()
{
    const std::filesystem::path assetPath = R"(C:\Projects\Build\raw)";
    const std::filesystem::path outPath = R"(C:\Projects\Build\res")";

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

    return 0;
}