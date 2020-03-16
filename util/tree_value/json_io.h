#pragma once

#include <util/tree_value/tree_value.h>

#include <istream>
#include <ostream>

class TJsonIO {
public:
    explicit TJsonIO(TTreeValue& value, bool pretty = false);

    explicit operator std::string() const;

    TJsonIO(const TJsonIO&) = delete;
    TJsonIO& operator=(const TJsonIO&) = delete;

private:
    TJsonIO(TTreeValue& value, std::int_least8_t level);

    [[nodiscard]]
    std::string ToString() const;

private:
    TTreeValue& Value;
    std::int_least8_t Pretty;

private:
    friend std::istream& operator>>(std::istream& stream, TJsonIO& json);
    friend std::istream& operator>>(std::istream& stream, TJsonIO&& json);
};

std::ostream& operator<<(std::ostream& stream, const TJsonIO& json);

TTreeValue BuildJson(std::string_view sv);
