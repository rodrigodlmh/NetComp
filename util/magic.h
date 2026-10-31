#pragma once

#ifndef NETCOMP_MAGIC_H
#define NETCOMP_MAGIC_H

#include <cstdint>

inline constexpr uint32_t MakeMagic(const uint8_t c1, const uint8_t c2, const uint8_t c3, const uint8_t c4)
{
    return (static_cast<uint32_t>(c4) << 24) | (static_cast<uint32_t>(c3) << 16) | (static_cast<uint32_t>(c2) << 8) | static_cast<uint32_t>(c1);
}

#endif //NETCOMP_MAGIC_H