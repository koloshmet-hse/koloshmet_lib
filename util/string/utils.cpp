#include "utils.h"

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

auto Split(std::string_view seq, std::string_view delim) {
    auto isDelim = [delim] (auto beg, auto end) {
        for (auto&& cur : delim) {
            if (beg == end) {
                return 1;
            }

            if (cur != *beg++) {
                return -1;
            }
        }

        if (beg == end) {
            return 0;
        }
        return -1;
    };
    return TSlice{std::move(seq), isDelim};
}
