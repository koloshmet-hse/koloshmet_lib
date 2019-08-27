#pragma once

#include <util/tree_value/tree_value.h>

#include <istream>
#include <ostream>

class TJsonIO {
public:
    explicit TJsonIO(TTreeValue& value);

    [[nodiscard]]
    std::string ToString() const;

private:
    TTreeValue& Value;

private:
    friend std::istream& operator>>(std::istream& stream, TJsonIO& json);
    friend std::istream& operator>>(std::istream& stream, TJsonIO&& json);
};

std::ostream& operator<<(std::ostream& stream, const TJsonIO& json);

TTreeValue BuildJson(std::string_view sv);
