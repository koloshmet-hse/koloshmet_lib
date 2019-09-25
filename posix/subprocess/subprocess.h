#pragma once

#include <posix/file_descriptor/fd_stream.h>

#include <filesystem>

#include <string>
#include <vector>
#include <optional>

namespace NInternal {
    std::pair<TUniqueFd, TUniqueFd> Pipe();
}

namespace {
    template <typename TType>
    struct TIsStringView {
    private:
        static constexpr void func(...);

        static constexpr char func(std::string_view);

    public:
        static constexpr bool value = !std::is_void_v<decltype(func(std::declval<TType>()))>;
    };

    template <typename TType>
    constexpr bool IsStringView = TIsStringView<TType>::value;
}

class TSubprocess {
public:
    template <typename... TArgs>
    explicit TSubprocess(const std::filesystem::path& executable, TArgs&&... args)
        : Executable(executable)
        , Arguments{executable.string()}
        , PreparedArgs{}
        , InStream{}
        , OutStream{}
        , ErrStream{}
        , InFds{NInternal::Pipe()}
        , OutFds{NInternal::Pipe()}
        , ErrFds{NInternal::Pipe()}
        , ChildPid{-1}
    {
        (Arguments.emplace_back(std::forward<TArgs>(args)), ...);
        for (auto&& arg : Arguments) {
            PreparedArgs.push_back(arg.data());
        }
        PreparedArgs.push_back(nullptr);
    }

    template <typename TIter>
    TSubprocess(
        const std::filesystem::path& executable,
        TIter beg,
        std::enable_if_t<!IsStringView<TIter>, TIter> end)
        : Executable(executable)
        , Arguments{executable.string()}
        , PreparedArgs{}
        , InStream{}
        , OutStream{}
        , ErrStream{}
        , InFds{NInternal::Pipe()}
        , OutFds{NInternal::Pipe()}
        , ErrFds{NInternal::Pipe()}
        , ChildPid{-1}
    {
        while (beg != end) {
            Arguments.emplace_back(*beg++);
        }
        for (auto&& arg : Arguments) {
            PreparedArgs.push_back(arg.data());
        }
        PreparedArgs.push_back(nullptr);
    }

    ~TSubprocess();

    void Execute();

    void Wait();

    void Kill();

    TOFdStream& In();

    TIFdStream& Out();

    TIFdStream& Err();

private:
    void ForkExec();

private:
    const std::filesystem::path Executable;
    std::vector<std::string> Arguments;
    std::vector<char*> PreparedArgs;
    std::optional<TOFdStream> InStream;
    std::optional<TIFdStream> OutStream;
    std::optional<TIFdStream> ErrStream;
    std::pair<TUniqueFd, TUniqueFd> InFds;
    std::pair<TUniqueFd, TUniqueFd> OutFds;
    std::pair<TUniqueFd, TUniqueFd> ErrFds;
    int ChildPid;
};
