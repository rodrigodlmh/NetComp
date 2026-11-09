#include "util/vfs.h"

void Vfs::AddPack(const std::filesystem::path& path, const std::string& name,
                  DiagnosticSink& diagnostics)
{
    if (std::ranges::any_of(packs_, [&](const PackInfo& p) { return p.name == name; }))
    {
        diagnostics.Error("Duplicate pack name: " + name, path);
        return;
    }
    if (!std::filesystem::is_directory(path))
    {
        diagnostics.Error("Pack directory not found", path);
        return;
    }

    const int index = static_cast<int>(packs_.size());
    packs_.push_back({path, name});

    std::vector<std::filesystem::path> files;
    std::error_code ec;
    for (const auto& e : std::filesystem::recursive_directory_iterator(path, ec))
        if (e.is_regular_file(ec)) files.push_back(e.path());
    if (ec) diagnostics.Error("Failed to scan pack: " + ec.message(), path);
    std::ranges::sort(files);

    for (const auto& file : files)
    {
        const std::string key = name + ":" + file.filename().string();
        const VfsEntry entry{file, key, GetFileType(file.extension().string()), index};

        if (!entries_.emplace(key, entry).second)
            diagnostics.Error("Duplicate asset: " + key, file);
    }
}

std::optional<std::filesystem::path> Vfs::GetAsset(const std::string& name) const
{
    auto it = entries_.find(name);
    if (it != entries_.end())
    {
        return it->second.path;
    }
    return std::nullopt;
}

std::optional<VfsEntry> Vfs::Find(const std::string& name) const
{
    auto it = entries_.find(name);
    return it != entries_.end() ? std::optional{it->second} : std::nullopt;
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
    std::string ext(extension);
    std::ranges::transform(ext, ext.begin(), [](unsigned char c) { return std::tolower(c); });
    if (ext == ".json") return FileType::JSON;
    if (ext == ".tsv") return FileType::TSV;
    if (ext == ".png") return FileType::PNG;
    if (ext == ".vert" || ext == ".frag" || ext == ".comp") return FileType::GLSL;
    return FileType::UNKNOWN;
}