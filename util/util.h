#pragma once

#ifndef NETCOMP_UTIL_H
#define NETCOMP_UTIL_H

#include <optional>
#include <unordered_map>
#include <string>

inline std::optional<uint32_t> LookupMap(const std::unordered_map<std::string, uint32_t>& map, const std::string& key)
{
    const auto it = map.find(key);
    if (it == map.end()) return std::nullopt;
    return it->second;
}

#endif //NETCOMP_UTIL_H