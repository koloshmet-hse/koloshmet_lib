#pragma once

#include <cstddef>

class TReserveType {
public:
    /* implicit */ constexpr TReserveType(std::size_t sz) noexcept
        : Reserved{sz}
    {}

    /* implicit */ constexpr operator std::size_t() const noexcept {
        return Reserved;
    }

private:
    std::size_t Reserved;
};

constexpr TReserveType operator""_rsrv(unsigned long long sz) {
    return TReserveType{sz};
}
