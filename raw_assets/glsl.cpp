#include "raw_assets/glsl.h"

#include <fstream>

std::optional<Spv> CompileGlslToSpv(const std::filesystem::path& path, const shaderc::Compiler& compiler, const shaderc::CompileOptions& options)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) return std::nullopt;

    shaderc_shader_kind kind;
    auto ext = path.extension();
    if (ext == ".vert") kind = shaderc_glsl_vertex_shader;
    else if (ext == ".frag") kind = shaderc_glsl_fragment_shader;
    else if (ext == ".comp") kind = shaderc_glsl_compute_shader;
    else return {};

    std::string source;
    const uintmax_t fileSize = std::filesystem::file_size(path);
    source.resize(fileSize);
    if (!file.read(source.data(), static_cast<std::streamsize>(fileSize))) return std::nullopt;

    const auto result = compiler.CompileGlslToSpv(source, kind, path.string().c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        return {};
    }

    Spv out;
    out.data.assign(result.cbegin(), result.cend());
    return out;
}