#pragma once

#include <string>
#include <string_view>

std::string UrlEncode(std::string_view str);

std::string UrlDecode(std::string_view str);
