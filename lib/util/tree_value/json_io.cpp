#include "json_io.h"

#include <util/string/utils.h>

#include <util/exception/exception.h>

#include <charconv>
#include <vector>

TJsonIO::TJsonIO(TTreeValue& value, bool pretty)
    : Value{value}
    , Pretty(pretty ? 1 : std::numeric_limits<std::int_least8_t>::lowest())
{}

TJsonIO::TJsonIO(TTreeValue& value, std::int_least8_t level)
    : Value{value}
    , Pretty{level}
{}

std::string TJsonIO::ToString() const {
    if (Value.Undefined()) {
        return {};
    }

    switch (Value.Value->index()) {
        case 0: {
            if (std::get<bool>(*Value.Value)) {
                return "true";
            } else {
                return "false";
            }
        }
        case 1:
            return std::to_string(std::get<long long>(*Value.Value));
        case 2:
            return std::to_string(std::get<double>(*Value.Value));
        case 3:
            return '"' + std::get<std::string>(*Value.Value) + '"';
        case 4: {
            std::string res;
            res += '[';
            for (auto&& json : std::get<TTreeValue::TArray>(*Value.Value)) {
                if (Pretty >= 0) {
                    res += '\n';
                    for (std::int_least8_t i = 0; i / 4 < Pretty; ++i) {
                        res += ' ';
                    }
                }
                res += TJsonIO{json, static_cast<std::int_least8_t>(Pretty + 1)}.ToString();
                res += ',';
            }
            if (std::get<TTreeValue::TArray>(*Value.Value).empty()) {
                res += ']';
            } else {
                if (Pretty >= 0) {
                    res.back() = '\n';
                    for (std::int_least8_t i = 0; i / 4 < Pretty - 1; ++i) {
                        res += ' ';
                    }
                    res += ']';
                } else {
                    res.back() = ']';
                }
            }
            return res;
        }
        case 5: {
            std::string res;
            res += '{';
            for (auto&& [label, json] : std::get<TTreeValue::TDict>(*Value.Value)) {
                if (Pretty >= 0) {
                    res += '\n';
                    for (std::int_least8_t i = 0; i / 4 < Pretty; ++i) {
                        res += ' ';
                    }
                }
                res += '"';
                res += label;
                res += '"';
                res += ':';
                if (Pretty >= 0) {
                    res += ' ';
                }
                res += TJsonIO{json, static_cast<std::int_least8_t>(Pretty + 1)}.ToString();
                res += ',';
            }
            if (std::get<TTreeValue::TDict>(*Value.Value).empty()) {
                res += '}';
            } else {
                if (Pretty >= 0) {
                    res.back() = '\n';
                    for (std::int_least8_t i = 0; i / 4 < Pretty - 1; ++i) {
                        res += ' ';
                    }
                    res += '}';
                } else {
                    res.back() = '}';
                }
            }
            return res;
        }
        default:
            return "";
    }
}

TJsonIO::operator std::string() const {
    return ToString();
}

namespace {
    template <typename TIterator>
    TIterator FindChar(TIterator beg, TIterator end, char c) {
        std::vector<char> stack;
        while (beg != end) {
            ++beg;
            if (*beg == '[' || *beg == '{') {
                stack.push_back(*beg);
            } else if (*beg == ']') {
                if (!stack.empty() && stack.back() == '[') {
                    stack.pop_back();
                } else {
                    return beg;
                }
            } else if (*beg == '}') {
                if (!stack.empty() && stack.back() == '{') {
                    stack.pop_back();
                } else {
                    return beg;
                }
            }
            if (*beg == c && stack.empty()) {
                return beg;
            }
        }
        return end;
    }

    template <typename TIterator>
    TIterator NextChar(TIterator first, TIterator last) {
        do {
            ++first;
        } while (std::isspace(*first) && first < last);
        return first;
    }

    template <typename TIterator>
    TIterator PrevChar(TIterator first, TIterator last) {
        do {
            --last;
        } while (std::isspace(*last) && first < last);
        return last;
    }

    template <typename TIterator>
    TIterator FindComma(TIterator beg, TIterator end) {
        return FindChar(beg, end, ',');
    }

    template <typename TIterator>
    TIterator FindColon(TIterator beg, TIterator end) {
        return FindChar(beg, end, ':');
    }

    template <typename TIterator>
    TIterator FindDot(TIterator beg, TIterator end) {
        return FindChar(beg, end, '.');
    }

}

template <typename TIterator>
TTreeValue BuildJsonHelper(TIterator first, TIterator last, std::string_view sv) {
    TTreeValue res;
    if (first >= last) {
        return res;
    }

    if (*first == '[' && *last == ']') {
        for (auto comma = FindComma(first, last); comma != last; comma = FindComma(first, last)) {
            res.Push(BuildJsonHelper(NextChar(first, comma), PrevChar(first, comma), sv));
            first = comma;
        }
        res.Push(BuildJsonHelper(NextChar(first, last), PrevChar(first, last), sv));
    } else if (*first == '{' && *last == '}') {
        for (auto comma = FindComma(first, last); comma != last; comma = FindComma(first, last)) {
            auto colon = FindColon(first, comma);

            auto beg = NextChar(first, colon);
            auto start = std::distance(sv.begin(), beg) + 1;
            auto len = std::distance(beg, PrevChar(first, colon)) - 1;

            res[std::string{sv.substr(start, len)}] = BuildJsonHelper(NextChar(colon, comma), PrevChar(colon, comma), sv);
            first = comma;
        }
        auto colon = FindColon(first, last);

        auto beg = NextChar(first, colon);
        auto start = std::distance(sv.begin(), beg) + 1;
        auto len = std::distance(beg, PrevChar(first, colon)) - 1;

        res[std::string{sv.substr(start, len)}] = BuildJsonHelper(NextChar(colon, last), PrevChar(colon, last), sv);
    } else if (*first == '"' && *last == '"') {
        auto start = std::distance(sv.begin(), std::next(first));
        auto len = std::distance(std::next(first), std::prev(last)) + 1;
        res = std::string{sv.substr(start, len)};
    } else if (std::isdigit(*first) && std::isdigit(*last)) {
        auto start = std::distance(sv.begin(), first);
        auto len = std::distance(first, last) + 1;
        auto number = sv.substr(start, len);
        if (FindDot(first, last) != last) {
            res = FromString<double>(number);
        } else {
            res = FromString<long long>(number);
        }
    } else {
        auto start = std::distance(sv.begin(), first);
        auto len = std::distance(first, last) + 1;
        auto obj = ToLower(sv.substr(start, len));
        if (obj == "true") {
            res = true;
        } else if (obj == "false") {
            res = false;
        } else {
            throw TException{"Bad format, not bool"};
        }
    }

    return res;
}

TTreeValue BuildJson(std::string_view sv) {
    auto first = sv.begin();
    auto last = std::prev(sv.end());
    if (std::isspace(*first)) {
        first = NextChar(first, last);
    }
    if (std::isspace(*last)) {
        last = PrevChar(first, last);
    }
    return BuildJsonHelper(first, last, sv);
}

std::istream& operator>>(std::istream& stream, TJsonIO& json) {
    std::string str(std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{});
    json.Value = BuildJson(str);
    return stream;
}

std::istream& operator>>(std::istream& stream, TJsonIO&& json) {
    std::string str(std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{});
    json.Value = BuildJson(str);
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const TJsonIO& json) {
    stream << static_cast<std::string>(json);
    return stream;
}
