#pragma once

#ifndef NETCOMP_HASH_H
#define NETCOMP_HASH_H

#include <cstdint>
#include <string_view>

constexpr uint64_t HashStr(const std::string_view s)
{
    uint64_t h = 0xcbf29ce484222325ull;
    for (const char c : s)
    {
        h ^= static_cast<uint8_t>(c);
        h *= 0x100000001b3ull;
    }
    return h;
}

#endif //NETCOMP_HASH_H