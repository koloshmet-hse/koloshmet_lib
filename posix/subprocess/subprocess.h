#pragma once

#include <posix/file_descriptor/fd_stream.h>
#include <posix/environ/environment_variable.h>

#include <filesystem>

#include <util/type/list_utils.h>
#include <util/exception/exception.h>

#include <string>
#include <vector>
#include <optional>

class TSubprocess {
private:
    template <typename TType>
    static constexpr bool IsStringView = std::is_convertible_v<TType, std::string_view>;

public:
    enum class EExitCode : long long {
        Ok = 0,
        Signaled = std::numeric_limits<long long>::max(),
        Unknown = std::numeric_limits<long long>::min()
    };

public:
    template <typename... TArgs>
    explicit TSubprocess(const std::filesystem::path& executable, TArgs&&... args)
            : Executable(executable)
            , Arguments{executable.string()}
            , EnvVars{}
            , PreparedArgs{}
            , PreparedEnv{}
            , InStream{}
            , OutStream{}
            , ErrStream{}
            , InFds{NInternal::Pipe()}
            , OutFds{NInternal::Pipe()}
            , ErrFds{NInternal::Pipe()}
            , ChildPid{-1}
    {
        if constexpr (FindType<TEnvVar, std::decay_t<TArgs>...>() == NPOS) {
            (Arguments.emplace_back(std::forward<TArgs>(args)), ...);
        } else {
            SeparateArgsFromEnvs(std::forward<TArgs>(args)...);
            PrepareEnvs();
        }
        PrepareArgs();
        Execute();
    }

    template <typename TIter, typename = TEmptyEnableIf<!IsStringView<TIter>>>
    TSubprocess(const std::filesystem::path& executable, TIter argBeg, TIter argEnd)
        : Executable(executable)
        , Arguments{executable.string()}
        , EnvVars{}
        , PreparedArgs{}
        , PreparedEnv{}
        , InStream{}
        , OutStream{}
        , ErrStream{}
        , InFds{NInternal::Pipe()}
        , OutFds{NInternal::Pipe()}
        , ErrFds{NInternal::Pipe()}
        , ChildPid{-1}
    {
        while (argBeg != argEnd) {
            Arguments.emplace_back(*argBeg++);
        }
        PrepareArgs();
        Execute();
    }

    template <typename TIter, typename TEnvIter, typename = TEmptyEnableIf<!IsStringView<TIter>>>
    TSubprocess(const std::filesystem::path& executable, TIter argBeg, TIter argEnd, TEnvIter envBeg, TEnvIter envEnd)
            : Executable(executable)
            , Arguments{executable.string()}
            , EnvVars{}
            , PreparedArgs{}
            , PreparedEnv{}
            , InStream{}
            , OutStream{}
            , ErrStream{}
            , InFds{NInternal::Pipe()}
            , OutFds{NInternal::Pipe()}
            , ErrFds{NInternal::Pipe()}
            , ChildPid{-1}
    {
        while (argBeg != argEnd) {
            Arguments.emplace_back(*argBeg++);
        }
        PrepareArgs();

        while (envBeg != envEnd) {
            EnvVars.emplace_back(*envBeg++);
        }
        PrepareEnvs();
        Execute();
    }

    TSubprocess(TSubprocess&& other) noexcept;

    TSubprocess& operator=(TSubprocess&& other) noexcept;

    ~TSubprocess();

    EExitCode Wait();

    void Kill();

    TOFdStream& In();

    TIFdStream& Out();

    TIFdStream& Err();

private:
    template <typename T>
    using IsEnvVar = std::is_same<std::decay_t<T>, TEnvVar>;

    template <typename TCur, typename... TOthers>
    void SeparateArgsFromEnvs(TCur&& cur, TOthers&&... others) {
        if constexpr (std::is_same_v<std::decay_t<TCur>, TEnvVar>) {
            static_assert(AllOf<IsEnvVar, TCur, TOthers...>(), "Pass env vars after args to subprocess");
            EnvVars.emplace_back(std::forward<TCur>(cur));
            EnvVars.emplace_back(std::forward<TOthers>(others)...);
        } else {
            Arguments.emplace_back(std::forward<TCur>(cur));
            SeparateArgsFromEnvs(std::forward<TCur>(cur), std::forward<TOthers>(others)...);
        }
    }

    void Execute();

    void ForkExec();

    void PrepareArgs();

    void PrepareEnvs();

private:
    std::filesystem::path Executable;
    std::vector<std::string> Arguments;
    std::vector<std::string> EnvVars;
    std::vector<char*> PreparedArgs;
    std::vector<char*> PreparedEnv;
    std::optional<TOFdStream> InStream;
    std::optional<TIFdStream> OutStream;
    std::optional<TIFdStream> ErrStream;
    std::pair<TUniqueFd, TUniqueFd> InFds;
    std::pair<TUniqueFd, TUniqueFd> OutFds;
    std::pair<TUniqueFd, TUniqueFd> ErrFds;
    int ChildPid;
};
