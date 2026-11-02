#include "raw_assets/tsv.h"

#include <fstream>

bool IsBlankLine(const std::string_view line)
{
    return line.find_first_not_of(" \t\r") == std::string_view::npos;
}

std::vector<std::string> SplitTabs(std::string_view line)
{
    std::vector<std::string> cells;
    size_t start = 0;
    for (;;)
    {
        const size_t tab = line.find('\t', start);
        if (tab == std::string_view::npos)
        {
            cells.emplace_back(line.substr(start));
            break;
        }
        cells.emplace_back(line.substr(start, tab - start));
        start = tab + 1;
    }
    return cells;
}

Tsv ReadTsv(const std::filesystem::path& path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};

    std::string text((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (text.starts_with("\xEF\xBB\xBF")) text.erase(0, 3); // BOM

    Tsv result;
    result.source = path;
    bool isHeader = true;

    int32_t lineNo = 0;
    size_t pos = 0;
    while (pos < text.size())
    {
        size_t end = text.find('\n', pos);
        if (end == std::string::npos) end = text.size();
        std::string_view line(text.data() + pos, end - pos);
        pos = end + 1;
        ++lineNo;

        if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
        if (IsBlankLine(line)) continue;

        auto cells = SplitTabs(line);
        if (isHeader) { result.headers = std::move(cells); isHeader = false; }
        else          { result.rows.push_back({ lineNo, std::move(cells) }); }
    }
    return result;
}