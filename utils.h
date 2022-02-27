#pragma once

#include <vector>
#include <string>

namespace utils {

std::vector<std::string> split(const std::string& text, const char splitter);

std::vector<char> first_char(const std::vector<std::string>& data);

}
