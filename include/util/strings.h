#pragma once

#include <string>
#include <vector>

std::vector<std::string> string_split(const std::string &str, char separator);
bool compare_case_insensitive(const std::string &lhs, const std::string &rhs);
bool string_starts_with(const std::string &string, const std::string &starts_with);
bool string_ends_with(const std::string &string, const std::string &ends_with);

