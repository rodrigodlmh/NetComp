#pragma once

#ifndef NETCOMP_BLOCK_DEF_H
#define NETCOMP_BLOCK_DEF_H

#include <cstdint>
#include <filesystem>
#include <vector>

#include "lang.h"
#include "texture.h"
#include "raw_assets/json.h"

enum Face : size_t { FaceUp, FaceDown, FaceNorth, FaceSouth, FaceEast, FaceWest, FaceCount };
enum class Geometry : uint32_t { Cube = 0 /*, Slab, Stairs, Cross ... */ };

struct BlockHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t blockCount;
};

struct BlockDef
{
    uint32_t name;
    uint32_t geometry;
    uint32_t faceTextures[FaceCount];
};

struct BlockDefs
{
    BlockHeader header;
    std::vector<BlockDef> defs;
};

std::optional<Geometry> ParseGeometry(const std::string& s);
std::optional<uint32_t> FindTexture(const nlohmann::json& t, const std::string& key, const TextureKeys& textureKeys, bool& ok);
std::optional<BlockDef> ParseBlock(const nlohmann::json& j, const LangKeys& langKeys, const TextureKeys& textureKeys);
void SaveBlockDefs(const std::filesystem::path& inPath, const LangKeys& langKeys, const TextureKeys& textureKeys, std::filesystem::path& outPath);

#endif //NETCOMP_BLOCK_DEF_H