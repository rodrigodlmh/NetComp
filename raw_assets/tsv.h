#pragma once

#ifndef NETCOMP_TSV_H
#define NETCOMP_TSV_H

#include <filesystem>
#include <string>
#include <vector>

struct TsvRow
{
    int32_t line;
    std::vector<std::string> cells;
};

struct Tsv
{
    std::filesystem::path source;
    std::vector<std::string> headers;
    std::vector<TsvRow> rows;
};

bool IsBlankLine(std::string_view line);
// Given a line of the TSV file, returns a vector of each cell divided by tabs
std::vector<std::string> SplitTabs(std::string_view line);
// Reads and parses a TSV file
Tsv ReadTsv(const std::filesystem::path& path);

#endif //NETCOMP_TSV_H