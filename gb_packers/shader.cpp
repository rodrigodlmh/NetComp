#include "gb_packers/shader.h"

#include <fstream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

ShaderJson ReadShaderJson(const std::filesystem::path& path, Diagnostics& diagnostics)
{
    std::ifstream file(path);
    if (!file) return {};

    const json j = json::parse(file);

    ShaderJson shader;
    shader.name = j.at("name").get<std::string>();
    shader.vertex = j.at("vertex").get<std::string>();
    shader.fragment = j.at("fragment").get<std::string>();
    shader.compute = j.at("compute").get<std::string>();

    if (shader.name.empty())
        diagnostics.Error(path, "Shader name is empty");
    if (shader.compute && (shader.vertex || shader.fragment))
        diagnostics.Error(path, "Shader contains compute but also vertex and fragment stages");
    if (!shader.vertex || !shader.fragment)
        diagnostics.Error(path, "Shader must contain both vertex and fragment if no compute is defined");
    return shader;
}
