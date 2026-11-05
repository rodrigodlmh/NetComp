#include "util/vfs.h"

void Vfs::AddPack(const std::filesystem::path& path, DiagnosticSink& diagnostics)
{
    PackInfo pack;
    pack.path = path;
    pack.name = path.stem().string();

    const int index = static_cast<int>(packs_.size());
    packs_.push_back(pack);

    std::filesystem::path roots[2];
    roots[0] = path / "assets";
    roots[1] = path / "data";

    for (const auto& root : roots)
    {
        if (!std::filesystem::exists(root)) continue;

        std::vector<std::filesystem::path> files;
        for (const auto& e : std::filesystem::recursive_directory_iterator(root))
            if (e.is_regular_file()) files.push_back(e.path());
        std::ranges::sort(files);

        for (const auto& file : files)
        {
            std::string name = pack.name + ":" + file.stem().string();
            VfsEntry entry{file, name, GetFileType(file.extension().string()), index};

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

std::vector<VfsEntry> Vfs::List(const FileType type)
{
    std::vector<VfsEntry> out;
    for (const auto& [name, e] : entries_)
    {
        if (e.type == type) out.push_back(e);
    }
    return out;
}

FileType GetFileType(const std::string_view extension)
{
    if (extension == ".json") return FileType::JSON;
    if (extension == ".tsv") return FileType::TSV;
    if (extension == ".png") return FileType::PNG;
    if (extension == ".vert" || extension == ".frag" || extension == ".comp") return FileType::GLSL;
    return FileType::UNKNOWN;
}