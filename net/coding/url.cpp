#include "url.h"

#include <sstream>
#include <iomanip>
#include <array>
#include <cctype>

bool IsSpecial(char c) {
    static constexpr std::array<char, 4> specials{'-', '_', '.', '~'};

    return std::find(specials.begin(), specials.end(), c) != specials.end();
};

std::string UrlEncode(std::string_view str) {
    std::ostringstream res;
    res << std::hex << std::uppercase << std::right << std::setfill('0');

    for (auto curChar : str) {
        if (std::isalpha(curChar) || std::isdigit(curChar) || IsSpecial(curChar)) {
            res << curChar;
        } else {
            res << '%' << std::setw(2) << static_cast<unsigned short>(curChar);
        }
    }

    return res.str();
}

std::string UrlDecode(std::string_view str) {
    std::string res;
    for (size_t index = 0; index < str.size(); ++index) {
        if (str[index] == '%') {
            std::istringstream stream(std::string{str.substr(index + 1, 2)});
            unsigned short num;
            stream >> std::hex >> num;
            res += static_cast<char>(num);
            index += 2;
        } else {
            res += str[index];
        }
    }
    return res;
}
