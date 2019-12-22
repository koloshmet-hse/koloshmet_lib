#pragma once

#include <cstddef>

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
    static constexpr std::size_t Size = sizeof...(TTypes);

    template <std::size_t Index>
    using TGet = typename NInternal::TTypeNode<Index, TTypes...>::type;
};
