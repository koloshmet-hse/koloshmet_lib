#pragma once

#include <lib/posix/file_descriptor/fd_stream.h>
#include <lib/posix/subprocess/environment_variable.h>

#include <filesystem>

#include <lib/util/type/list_utils.h>
#include <lib/util/exception/exception.h>

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

    enum class ECommunicationMode : std::uint_least32_t {
        Direct = 0,
        In = 1,
        InPt = 3,
        Out = 4,
        OutPt = 12,
        Err = 16,
        ErrPt = 48,
        Pt = 42
    };

public:
    template <typename... TArgs>
    explicit TSubprocess(
        std::filesystem::path executable,
        ECommunicationMode communicationMode,
        TArgs&&... args)
        : Executable(FindExecutablePath(std::move(executable)))
        , Arguments{Executable.string()}
        , EnvVars{}
        , PreparedArgs{}
        , PreparedEnv{}
        , InStream{}
        , OutStream{}
        , ErrStream{}
        , ChildPid{-1}
    {
        if constexpr (FindType<NSubprocess::TEnvVar, std::decay_t<TArgs>...>() == NPOS) {
            (Arguments.emplace_back(std::forward<TArgs>(args)), ...);
        } else {
            SeparateArgsFromEnvs(std::forward<TArgs>(args)...);
            PrepareEnvs();
        }
        PrepareArgs();
        ForkExec(communicationMode);
    }

    template <typename TIter, typename = TEmptyEnableIf<!IsStringView<TIter>>>
    TSubprocess(
        std::filesystem::path executable,
        ECommunicationMode communicationMode,
        TIter argBeg, TIter argEnd)
        : Executable(FindExecutablePath(std::move(executable)))
        , Arguments{Executable.string()}
        , EnvVars{}
        , PreparedArgs{}
        , PreparedEnv{}
        , InStream{}
        , OutStream{}
        , ErrStream{}
        , ChildPid{-1}
    {
        while (argBeg != argEnd) {
            Arguments.emplace_back(*argBeg++);
        }
        PrepareArgs();
        ForkExec(communicationMode);
    }

    template <typename TIter, typename TEnvIter, typename = TEmptyEnableIf<!IsStringView<TIter>>>
    TSubprocess(
        std::filesystem::path executable,
        ECommunicationMode communicationMode,
        TIter argBeg, TIter argEnd,
        TEnvIter envBeg, TEnvIter envEnd)
        : Executable(FindExecutablePath(std::move(executable)))
        , Arguments{Executable.string()}
        , EnvVars{}
        , PreparedArgs{}
        , PreparedEnv{}
        , InStream{}
        , OutStream{}
        , ErrStream{}
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
        ForkExec(communicationMode);
    }

    TSubprocess(TSubprocess&& other) noexcept;

    TSubprocess& operator=(TSubprocess&& other) noexcept;

    ~TSubprocess();

    EExitCode Wait();

    void Kill();

    TOFdStream& In();

    TIFdStream& Out();

    TIFdStream& Err();

    static std::filesystem::path FindExecutablePath(std::filesystem::path executable);

private:
    template <typename T>
    using IsEnvVar = std::is_same<std::decay_t<T>, NSubprocess::TEnvVar>;

    template <typename TCur, typename... TOthers>
    void SeparateArgsFromEnvs(TCur&& cur, TOthers&&... others) {
        if constexpr (std::is_same_v<std::decay_t<TCur>, NSubprocess::TEnvVar>) {
            static_assert(AllOf<IsEnvVar, TCur, TOthers...>(), "Pass env vars after args to subprocess");
            EnvVars.emplace_back(std::forward<TCur>(cur));
            EnvVars.emplace_back(std::forward<TOthers>(others)...);
        } else {
            Arguments.emplace_back(std::forward<TCur>(cur));
            SeparateArgsFromEnvs(std::forward<TCur>(cur), std::forward<TOthers>(others)...);
        }
    }

    void ForkExec(ECommunicationMode mode);

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

    int ChildPid;
};

bool operator&(TSubprocess::ECommunicationMode l, TSubprocess::ECommunicationMode r);

TSubprocess::ECommunicationMode operator|(TSubprocess::ECommunicationMode l, TSubprocess::ECommunicationMode r);
