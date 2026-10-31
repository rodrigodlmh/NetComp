#pragma once

#ifndef NETCOMP_DIAGNOSTICS_H
#define NETCOMP_DIAGNOSTICS_H

#include <filesystem>
#include <string>
#include <vector>

struct Diagnostics
{
    std::vector<std::string> errors;

    void Error(const std::filesystem::path& file, const std::string& msg, const uint32_t line = 0)
    {
        errors.push_back(file.string() + ":" + std::to_string(line) + ": " + msg);
    }
    [[nodiscard]] bool Ok() const { return errors.empty(); }
};

#endif //NETCOMP_DIAGNOSTICS_H