#pragma once

#ifndef NETCOMP_JSON_H
#define NETCOMP_JSON_H

#include <optional>

#include <nlohmann/json.hpp>

#include "util/diagnostics.h"

std::optional<nlohmann::json> LoadJson(const std::filesystem::path& path, DiagnosticSink& diagnostics);
bool ReadOptional(const nlohmann::json& j, const char* key,
                  std::optional<std::string>& out,
                  const std::filesystem::path& path, DiagnosticSink& diagnostics);

#endif //NETCOMP_JSON_H