#pragma once

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
