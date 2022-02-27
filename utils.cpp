#include "utils.h"

std::vector<std::string> utils::split(const std::string& text, const char splitter)
{
    std::size_t offset = 0;
    std::vector<std::string> result;
    while(true) {
        const auto splitter_pos = text.find(splitter, offset);
        if (splitter_pos == std::string::npos) break;
        if (splitter_pos != offset)
            result.push_back(text.substr(offset, splitter_pos - offset));
        offset = splitter_pos + 1;
    }
    if (result.size() != offset)
        result.push_back(text.substr(offset, result.size() - offset));
    return result;
}

std::vector<char> utils::first_char(const std::vector<std::string>& data)
{
    std::vector<char> result;
    result.reserve(data.size());
    for (const auto& el : data) {
        if (el.empty()) continue;
        result.push_back(el.front());
    }
    return result;
}
