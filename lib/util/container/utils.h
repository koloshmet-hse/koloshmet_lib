#pragma once

#include <util/container/slice.h>

template <typename TContainer, typename TDelimiter, typename TContainerHolder, typename TDelimiterHolder>
TSlice<TContainer, TDelimiter, TContainerHolder, TDelimiterHolder> Slice(TContainerHolder&& container, TDelimiterHolder&& delimiter) {
    return {std::forward<TContainer>(container), std::forward<TContainer>(delimiter)};
}
