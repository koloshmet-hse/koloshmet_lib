#pragma once

#include <cstddef>
#include <array>

#include <lib/util/global/constants.h>

namespace NInternal {
    template <std::size_t Index, typename... TTypes>
    struct TTypeNode;

    template <std::size_t Index>
    struct TTypeNode<Index> {
        using type = void;
    };

    template <typename T, typename... TTypes>
    struct TTypeNode<0, T, TTypes...> {
        using type = T;
    };

    template <std::size_t Index, typename T, typename... TTypes>
    struct TTypeNode<Index, T, TTypes...> : TTypeNode<Index - 1, TTypes...> {};
}

template <typename... TTypes>
struct TTypeList {
    static constexpr std::size_t Size() noexcept {
        return sizeof...(TTypes);
    }

    template <typename T>
    static constexpr std::size_t Find() noexcept {
        constexpr std::array<bool, sizeof...(TTypes)> isSames{std::is_same_v<T, TTypes>...};
        for (std::size_t i = 0; i < isSames.size(); ++i) {
            if (isSames[i]) {
                return i;
            }
        }
        return NPOS;
    }

    template <std::size_t Index>
    using TGet = typename NInternal::TTypeNode<Index, TTypes...>::type;
};
