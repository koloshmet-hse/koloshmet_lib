#pragma once

#include <util/containers/slice.h>
#include <charconv>

#include <string>
#include <string_view>

std::string ToLower(std::string_view);

std::string ToUpper(std::string_view);

template <typename T>
std::string ToString(const T& t) {
    if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
        return std::to_string(t);
    } else {
        return static_cast<std::string>(t);
    }
}

template <typename T, typename... TParams>
T FromString(std::string_view str, TParams&&... param) {
    if constexpr (std::is_integral_v<T>) {
        T res{};
        std::from_chars(str.data(), str.data() + str.size(), res, std::forward<TParams>(param)...);
        return res;
    } else if (std::is_floating_point_v<T>) {
        if constexpr (std::is_same_v<T, float>) {
            return std::stof(std::string{str});
        } else if (std::is_same_v<T, double>) {
            return std::stod(std::string{str});
        } else {
            return std::stold(std::string{str});
        }
    } else {
        return T(str);
    }
}

template <>
struct TSlicer<std::string_view> {
    template <typename TIter>
    std::string_view operator()(
        TIter begin,
        TIter end,
        std::string_view sequence) const
    {
        auto first = std::distance(sequence.begin(), begin);
        auto len = std::distance(begin, end);
        return sequence.substr(first, len);
    }
};

namespace NInternal {
    struct TIsDelim {
        template <typename TIter>
        int operator()(TIter beg, TIter end) {
            for (auto&& cur : Delim) {
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
        }

        std::string_view Delim;
    };
}

TSlice<std::string_view, NInternal::TIsDelim> Split(std::string_view seq, std::string_view delim);
