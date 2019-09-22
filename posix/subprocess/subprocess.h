#pragma once

#include <posix/file_descriptor/fd_stream.h>

#include <filesystem>

#include <string>
#include <vector>
#include <optional>

namespace NInternal {
    std::pair<TUniqueFd, TUniqueFd> Pipe();
}

class TSubprocess {
public:
    template <typename... TArgs>
    explicit TSubprocess(const std::filesystem::path& executable, TArgs&&... args)
        : Executable(executable)
        , Arguments{executable.string(), std::forward<TArgs>(args)...}
        , PreparedArgs{}
        , InStream{}
        , OutStream{}
        , ErrStream{}
        , InFds{NInternal::Pipe()}
        , OutFds{NInternal::Pipe()}
        , ErrFds{NInternal::Pipe()}
        , ChildPid{-1}
    {
        for (auto&& arg : Arguments) {
            PreparedArgs.push_back(arg.data());
        }
        PreparedArgs.push_back(nullptr);
    }

    ~TSubprocess();

    void Execute();

    void Wait();

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
