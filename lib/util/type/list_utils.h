#pragma once

#include <util/type/type_list.h>

#include <utility>

template <std::size_t Index, typename... TArgs>
using TGetType = typename TTypeList<TArgs...>::template TGet<Index>;

template <typename T, typename... TArgs>
constexpr std::size_t FindType() noexcept {
    return TTypeList<TArgs...>::template Find<T>();
}

namespace NInternal {
    template <template <typename> typename TPredicate, typename... TArgs, std::size_t... Indexes>
    constexpr bool AllOfImpl(std::index_sequence<Indexes...>) {
        bool res = true;
        return (res &= ... &= TPredicate<TGetType<Indexes, TArgs...>>::value);
    }
}

template <template <typename> typename TPredicate, typename... TArgs>
constexpr bool AllOf() noexcept {
    return NInternal::AllOfImpl<TPredicate, TArgs...>(std::index_sequence_for<TArgs...>{});
}
