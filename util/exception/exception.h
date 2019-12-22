#pragma once

#include <exception>

#include <sstream>
#include <string_view>

#include <util/type/utils.h>

class TException : public std::exception {
    template <typename T>
    static constexpr bool IsCStr = std::is_same_v<std::decay_t<T>, const char*>;

    template <typename T>
    static constexpr bool IsArray = std::is_array_v<std::remove_reference_t<T>>;

public:
    TException()
        : std::exception{}
        , Owner{}
        , Message{nullptr}
    {}

    template <size_t N>
    explicit constexpr TException(const char (&msg)[N])
        : std::exception{}
        , Owner{}
        , Message{msg}
    {}

    template <typename TArg>
    explicit TException(TArg&& arg, TEmptyEnableIf<!IsCStr<TArg> || !IsArray<TArg>> = nullptr)
        : std::exception{}
        , Owner{}
        , Message{nullptr}
    {
        std::ostringstream stream;
        stream << std::forward<TArg>(arg);
        Owner = stream.str();
    }

    template <typename TArg1, typename TArg2, typename... TArgs>
    explicit TException(TArg1&& arg1, TArg2&& arg2, TArgs&&... args)
        : std::exception{}
        , Owner{}
        , Message{nullptr}
    {
        std::ostringstream stream;
        stream << std::forward<TArg1>(arg1) << std::forward<TArg2>(arg2);
        Ignore((stream << ... << std::forward<TArgs>(args)));
        Owner = stream.str();
    }

    TException(const TException& exception) noexcept;

    TException& operator=(const TException& exception) noexcept;

    [[nodiscard]]
    const char* what() const noexcept override;

private:
    std::string Owner;
    const char* Message;
};
