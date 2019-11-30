#pragma once

#include <cstddef>

class TReserveType {
public:
    /* implicit */ constexpr TReserveType(size_t sz) noexcept
        : Reserved{sz}
    {}

    /* implicit */ constexpr operator size_t() const noexcept {
        return Reserved;
    }

private:
    size_t Reserved;
};

constexpr TReserveType operator""_rsrv(unsigned long long sz) {
    return TReserveType{sz};
}
