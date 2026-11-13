#pragma once

#ifndef NETCOMP_SHADER_H
#define NETCOMP_SHADER_H

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>
#include <shaderc/shaderc.hpp>

struct ShaderHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t vertSize;
    uint32_t fragSize;
    uint32_t compSize;
};

struct Shader
{
    ShaderHeader header;
    std::vector<uint32_t> vertData;
    std::vector<uint32_t> fragData;
    std::vector<uint32_t> compData;
};

void SaveShader(const shaderc::Compiler& compiler, const std::filesystem::path& shadersPath, const std::string& name, const std::filesystem::path& outPath);

#endif //NETCOMP_SHADER_H