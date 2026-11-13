#include "bin_packers/shader.h"

#include <fstream>
#include <iostream>

#include "raw_assets/glsl.h"
#include "util/magic.h"

void SaveShader(const shaderc::Compiler& compiler, const std::filesystem::path& shadersPath, const std::string& name, const std::filesystem::path& outPath)
{
    if (!std::filesystem::is_directory(shadersPath))
    {
        std::cerr << "Shader path must be a directory: " << shadersPath << '\n';
        return;
    }

    std::vector<std::filesystem::path> shaders;

    for (const auto& file : std::filesystem::directory_iterator(shadersPath))
    {
        if (file.is_regular_file() && file.path().stem() == name) shaders.push_back(file.path());
    }

    if (shaders.size() != 1 && shaders.size() != 2)
    {
        std::cerr << "Expected 1 or 2 shaders named: " << name << '\n';
        return;
    }

    Shader shader{};
    shader.header.magic = MakeMagic('S', 'H', 'D', 'R');
    shader.header.version = 1;
    shader.header.compSize = 0;
    shader.header.vertSize = 0;
    shader.header.fragSize = 0;

    shaderc::CompileOptions options;
    options.SetWarningsAsErrors();
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);

    if (shaders.size() == 1)
    {
        if (shaders[0].extension() != ".comp")
        {
            std::cerr << "Individual shaders must be compute shaders (.comp)\n";
            return;
        }

        auto comp = CompileGlslToSpv(shaders[0], compiler, options);

        if (!comp) return;

        shader.compData = std::move(comp->data);
        shader.header.compSize = static_cast<uint32_t>(shader.compData.size());
    }
    else
    {
        std::filesystem::path vertPath;
        std::filesystem::path fragPath;

        for (const auto& path : shaders)
        {
            if (path.extension() == ".vert") vertPath = path;
            else if (path.extension() == ".frag") fragPath = path;
        }

        if (vertPath.empty() || fragPath.empty())
        {
            std::cerr << "Expected a vertex and fragment shader pair\n";
            return;
        }

        auto vert = CompileGlslToSpv(vertPath, compiler, options);
        auto frag = CompileGlslToSpv(fragPath, compiler, options);

        if (!vert || !frag) return;

        shader.vertData = std::move(vert->data);
        shader.fragData = std::move(frag->data);
        shader.header.vertSize = static_cast<uint32_t>(shader.vertData.size());
        shader.header.fragSize = static_cast<uint32_t>(shader.fragData.size());
    }

    std::ofstream file(outPath / (name + ".bin"), std::ios::binary);
    if (!file)
    {
        std::cerr << "Could not create shader file\n";
        return;
    }

    // Write header
    file.write(reinterpret_cast<const char*>(&shader.header), sizeof(ShaderHeader));

    if (!shader.vertData.empty())
    {
        file.write(reinterpret_cast<const char*>(shader.vertData.data()), static_cast<std::streamsize>(shader.vertData.size() * sizeof(uint32_t)));
    }
    if (!shader.fragData.empty())
    {
        file.write( reinterpret_cast<const char*>(shader.fragData.data()), static_cast<std::streamsize>(shader.fragData.size() * sizeof(uint32_t)));
    }
    if (!shader.compData.empty())
    {
        file.write( reinterpret_cast<const char*>(shader.compData.data()), static_cast<std::streamsize>(shader.compData.size() * sizeof(uint32_t)));
    }

    if (!file)
    {
        std::cout << "Failed writing shader file\n";
    }
    else
    {
        std::cout << "Saved shader " << name << "\n";
    }
}