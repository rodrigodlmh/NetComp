#pragma once

#ifndef NETCOMP_SHADER_H
#define NETCOMP_SHADER_H

#include <string>
#include <optional>

#include "raw_assets/glsl.h"
#include "util/diagnostics.h"

struct ShaderJson
{
    std::string name;
    std::optional<std::string> vertex;
    std::optional<std::string> fragment;
    std::optional<std::string> compute;
};

ShaderJson ReadShaderJson(const std::filesystem::path& path, DiagnosticSink& diagnostics);

#endif //NETCOMP_SHADER_H