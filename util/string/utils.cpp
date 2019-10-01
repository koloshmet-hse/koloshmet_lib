#include "utils.h"

#include <algorithm>
#include <cctype>

std::string ToLower(std::string_view str) {
    std::string res;
    auto transformer = [](const char& c) { return std::tolower(c); };
    std::transform(str.begin(), str.end(), std::back_inserter(res), transformer);
    return res;
}

std::string ToUpper(std::string_view str) {
    std::string res;
    auto transformer = [](const char& c) { return std::toupper(c); };
    std::transform(str.begin(), str.end(), std::back_inserter(res), transformer);
    return res;
}

TSlice<std::string_view, NInternal::TIsDelim> Split(std::string_view seq, std::string_view delim) {
    return TSlice{std::move(seq), NInternal::TIsDelim{delim}};
}
