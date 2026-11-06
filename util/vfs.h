#pragma once

#ifndef NETCOMP_VFS_H
#define NETCOMP_VFS_H

#include <string>
#include <filesystem>
#include <map>

#include "util/diagnostics.h"

enum class FileType
{
    UNKNOWN = 0,

    // Assets
    TSV, PNG, GLSL,

    // Data
    JSON,
};

struct PackInfo
{
    std::filesystem::path path;
    std::string name;
};

struct VfsEntry
{
    std::filesystem::path path;
    std::string name;
    FileType type;
    int packIndex = 0;
};

class Vfs
{
public:

    void AddPack(const std::filesystem::path& path, DiagnosticSink& diagnostics);
    [[nodiscard]] std::optional<std::filesystem::path> GetAsset(const std::string& name) const;
    std::vector<VfsEntry> List(FileType type);

private:

    std::vector<PackInfo> packs_;
    std::map<std::string, VfsEntry> entries_;
};

FileType GetFileType(std::string_view extension);

#endif //NETCOMP_VFS_H