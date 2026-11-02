#pragma once

#ifndef NETCOMP_GLSL_H
#define NETCOMP_GLSL_H

#include <filesystem>
#include <vector>

#include <shaderc/shaderc.hpp>

struct Spv
{
    std::vector<uint32_t> data;
};

Spv CompileGlslToSpv(const std::filesystem::path& path, const shaderc::Compiler& compiler, const shaderc::CompileOptions& options);

#endif //NETCOMP_GLSL_H