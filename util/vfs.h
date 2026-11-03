#pragma once

#ifndef NETCOMP_VFS_H
#define NETCOMP_VFS_H

#include <string>
#include <filesystem>
#include <map>

#include "util/hash.h"

struct AssetId
{
    std::string ns;
    std::string name;

    [[nodiscard]] std::string Str() const {return ns + ":" + name;}
};

struct AssetIdHash
{
    size_t operator()(const AssetId& id) const noexcept { return HashStr(id.Str()); }
};

struct PackInfo
{
    std::string id;
    std::string ns; // Namespace
    std::string version;
    std::filesystem::path root;
};

struct VfsEntry
{
    AssetId id;
    std::filesystem::path path;
    int packIndex = 0;
    bool isData = false;
};

class Vfs
{
public:



private:

    std::vector<PackInfo> packs;
    std::map<AssetId, VfsEntry> entries;
};

#endif //NETCOMP_VFS_H