#pragma once

#ifndef NETCOMP_GLSL_H
#define NETCOMP_GLSL_H

#include <filesystem>
#include <vector>

#include <shaderc/shaderc.hpp>

struct RawSpv
{
    std::vector<uint32_t> data;
};

class ShaderCompiler
{
public:

    ShaderCompiler();

private:

    shaderc::Compiler compiler_;
    shaderc::CompileOptions options_;

public:

    [[nodiscard]] RawSpv CompileGlslToSpv(const std::filesystem::path& path) const;
};

#endif //NETCOMP_GLSL_H