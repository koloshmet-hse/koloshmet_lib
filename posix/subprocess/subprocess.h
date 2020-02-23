#pragma once

#include <posix/file_descriptor/fd_stream.h>

#include <filesystem>

#include <util/exception/exception.h>

#include <string>
#include <vector>
#include <optional>

class TSubprocess {
public:
    template <typename TType>
    static constexpr bool IsStringView = std::is_convertible_v<TType, std::string_view>;

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
        CheckExecutable();
        (Arguments.emplace_back(std::forward<TArgs>(args)), ...);
        for (auto&& arg : Arguments) {
            PreparedArgs.push_back(arg.data());
        }
        PreparedArgs.push_back(nullptr);
    }


    template <typename TContainer, typename = TEmptyEnableIf<!IsStringView<TContainer>>>
    explicit TSubprocess(const std::filesystem::path& executable, TContainer&& args)
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
        CheckExecutable();
        for (auto&& arg : args) {
            Arguments.emplace_back(std::move(arg));
        }
        for (auto&& arg : Arguments) {
            PreparedArgs.push_back(arg.data());
        }
        PreparedArgs.push_back(nullptr);
    }

    template <typename TContainer, typename TEnvContainer, typename = TEmptyEnableIf<!IsStringView<TContainer>>>
    explicit TSubprocess(const std::filesystem::path& executable, TContainer&& args, TEnvContainer&& envs)
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
        CheckExecutable();
        for (auto&& arg : args) {
            Arguments.emplace_back(std::forward<decltype(arg)>(arg));
        }
        for (auto&& arg : Arguments) {
            PreparedArgs.push_back(arg.data());
        }

        for (auto&& env : envs) {
            EnvVars.emplace_back(std::forward<decltype(env)>(env));
        }
        PreparedArgs.push_back(nullptr);
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
        CheckExecutable();
        while (argBeg != argEnd) {
            Arguments.emplace_back(*argBeg++);
        }
        for (auto&& arg : Arguments) {
            PreparedArgs.push_back(arg.data());
        }
        PreparedArgs.push_back(nullptr);
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
        CheckExecutable();
        while (argBeg != argEnd) {
            Arguments.emplace_back(*argBeg++);
        }
        for (auto&& arg : Arguments) {
            PreparedArgs.push_back(arg.data());
        }
        PreparedArgs.push_back(nullptr);

        while (envBeg != envEnd) {
            EnvVars.emplace_back(*envBeg++);
        }
        for (auto&& env : EnvVars) {
            PreparedEnv.push_back(env.data());
        }
        PreparedEnv.push_back(nullptr);
    }

    TSubprocess(TSubprocess&& other) noexcept;

    TSubprocess& operator=(TSubprocess&& other) noexcept;

    ~TSubprocess();

    void Execute();

    void Wait();

    void Kill();

    TOFdStream& In();

    TIFdStream& Out();

    TIFdStream& Err();

    void AddEnv(std::string var, std::string_view value);

    void ClearEnv();

private:
    void ForkExec();

    void CheckExecutable() const;

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
