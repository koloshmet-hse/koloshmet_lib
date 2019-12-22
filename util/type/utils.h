#pragma once

#include <type_traits>

template <typename T>
constexpr void Ignore(const T&) {}

template <bool Predicate>
using TEmptyEnableIf = std::enable_if_t<Predicate, std::nullptr_t>;
