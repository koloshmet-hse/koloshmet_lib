#pragma once

#include <string_view>

#include <util/type/list_utils.h>
#include <util/exception/exception.h>

namespace NInternal {
    template <typename T>
    using TIsOptionType = std::disjunction<
        std::is_same<T, long long>,
        std::is_same<T, long double>,
        std::is_same<T, bool>,
        std::is_same<T, std::string_view>
    >;

    template <typename T>
    using TIsParameterType = std::disjunction<
        std::is_same<T, long long>,
        std::is_same<T, long double>,
        std::is_same<T, std::string_view>
    >;

    template <typename T>
    constexpr bool IsOptionType = TIsOptionType<T>::value;

    template <typename T>
    constexpr bool IsParameterType = TIsParameterType<T>::value;
}

template <typename T>
struct TOpt {
    static_assert(NInternal::IsOptionType<T>);

    using TType = T;

    explicit constexpr TOpt(std::string_view key, std::string_view description = "")
        : Key{key}
        , Description{description}
        , Default{}
        , Required{true}
    {
        if constexpr (std::is_arithmetic_v<T>) {
            Default = 0;
            Required = false;
        }
    }

    constexpr TOpt(std::string_view key, std::string_view description, T def)
        : Key{key}
        , Description{description}
        , Default{def}
        , Required{false}
    {}

    TOpt(const TOpt&) = default;

    TOpt& operator=(const TOpt&) = default;

    std::string_view Key;
    std::string_view Description;
    TType Default;
    bool Required;
};

template <typename... TParams>
struct TParamList {
    static_assert(AllOf<NInternal::TIsParameterType, TParams...>);

    using TTypes = TTypeList<TParams...>;

    constexpr TParamList(std::initializer_list<std::string_view> names)
        : Names{names}
    {
        if (names.size() != sizeof...(TParams)) {
            throw TException{"All params must be named"};
        }
    }

    std::initializer_list<std::string_view> Names;
};

template <typename TParam>
struct TDefaultParam {
    static_assert(NInternal::IsParameterType<TParam>);

    using TType = TParam;

    explicit constexpr TDefaultParam(std::string_view name)
        : Name{name}
    {}

    std::string_view Name;
};
