#pragma once

#ifndef NETCOMP_VFS_H
#define NETCOMP_VFS_H

#include <string>
#include <filesystem>
#include <map>

#include "util/diagnostics.h"

struct PackInfo
{
    std::filesystem::path path;
    std::string name;
};

struct VfsEntry
{
    std::filesystem::path path;
    int packIndex = 0;
    bool isData = false;
};

class Vfs
{
public:

    void AddPack(const std::filesystem::path& path, DiagnosticSink& diagnostics);

private:

    std::vector<PackInfo> packs_;
    std::map<std::string, VfsEntry> entries_;
};

#endif //NETCOMP_VFS_H