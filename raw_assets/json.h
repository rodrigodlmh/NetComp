#pragma once

#ifndef NETCOMP_JSON_H
#define NETCOMP_JSON_H

#include <optional>

#include <nlohmann/json.hpp>

std::optional<nlohmann::json> LoadJson(const std::filesystem::path& path);

#endif //NETCOMP_JSON_H