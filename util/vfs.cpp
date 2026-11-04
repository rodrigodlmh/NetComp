#include "util/vfs.h"

#include <fstream>

#include <nlohmann/json.hpp>

void Vfs::AddPack(const std::filesystem::path& path, DiagnosticSink& diagnostics)
{
    PackInfo pack;
    pack.path = path;
    pack.name = path.stem().string();

    const int index = static_cast<int>(packs_.size());
    packs_.push_back(pack);

    struct Root { const char* folder; bool isDef; };
    constexpr Root roots[] = {{"assets", false}, {"data", true}};

    for (const auto& root : roots)
    {
        std::filesystem::path rootDir = path / root.folder;
        if (!std::filesystem::exists(rootDir)) continue;

        std::vector<std::filesystem::path> files;
        for (const auto& e : std::filesystem::recursive_directory_iterator(rootDir))
            if (e.is_regular_file()) files.push_back(e.path());
        std::ranges::sort(files);

        for (const auto& file : files)
        {
            if (root.isDef && file.extension() != ".json")
            {
                diagnostics.Warning("Ignoring non-json file inside data/", file);
                continue;
            }
            VfsEntry entry{file, index, root.isDef};
            std::string name = pack.name + ":" + file.stem().string();

            auto [it, inserted] = entries_.emplace(name, entry);
            if (!inserted)
            {
                if (it->second.packIndex == index)
                {
                    diagnostics.Error("Duplicate asset: " + name);
                }
                else
                {
                    diagnostics.Info(name + " overrides " + packs_[it->second.packIndex].name, file);
                    it->second = entry;
                }
            }
        }
    }
}