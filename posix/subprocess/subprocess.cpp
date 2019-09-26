#include "subprocess.h"

#include <unistd.h>
#include <sys/wait.h>
#include <csignal>

#include <array>
#include <util/exception/exception.h>

std::pair<TUniqueFd, TUniqueFd> NInternal::Pipe() {
    std::array<int, 2> fds{};
    if (pipe(fds.data()) < 0) {
        throw std::system_error{std::error_code{errno, std::system_category()}};
    }
    return {TUniqueFd{fds[0]}, TUniqueFd{fds[1]}};
}

TSubprocess::TSubprocess(TSubprocess&& other) noexcept
    : Executable(std::move(other.Executable))
    , Arguments(std::move(other.Arguments))
    , PreparedArgs(std::move(other.PreparedArgs))
    , InStream(std::move(other.InStream))
    , OutStream(std::move(other.OutStream))
    , ErrStream(std::move(other.ErrStream))
    , InFds(std::move(other.InFds))
    , OutFds(std::move(other.OutFds))
    , ErrFds(std::move(other.ErrFds))
    , ChildPid(other.ChildPid)
{
    other.ChildPid = -1;
}

TSubprocess& TSubprocess::operator=(TSubprocess&& other) noexcept {
    Kill();
    Executable = std::move(other.Executable);
    Arguments = std::move(other.Arguments);
    PreparedArgs = std::move(other.PreparedArgs);
    InStream = std::move(other.InStream);
    OutStream = std::move(other.OutStream);
    ErrStream = std::move(other.ErrStream);
    InFds = std::move(other.InFds);
    OutFds = std::move(other.OutFds);
    ErrFds = std::move(other.ErrFds);
    ChildPid = other.ChildPid;
    other.ChildPid = -1;
    return *this;
}

TSubprocess::~TSubprocess() {
    Kill();
}

void TSubprocess::Execute() {
    ForkExec();
    if (InFds.second) {
        InStream.emplace(std::move(InFds.second));
    }
    if (OutFds.first) {
        OutStream.emplace(std::move(OutFds.first));
    }
    if (ErrFds.first) {
        ErrStream.emplace(std::move(ErrFds.first));
    }
}

void TSubprocess::Wait() {
    int status;
    InStream->Close();
    if (ChildPid > 0 && waitpid(ChildPid, std::addressof(status), 0) < 0) {
        throw std::system_error{std::error_code{errno, std::system_category()}};
    }
    ChildPid = -1;
}

void TSubprocess::Kill() {
    if (ChildPid > 0) {
        kill(ChildPid, SIGKILL);
    }
}

TOFdStream& TSubprocess::In() {
    if (InStream && InStream->IsOpen()) {
        return *InStream;
    }
    throw TException("No valid input fd");
}

TIFdStream& TSubprocess::Out() {
    if (OutStream && OutStream->IsOpen()) {
        return *OutStream;
    }
    throw TException("No valid output fd");
}

TIFdStream& TSubprocess::Err() {
    if (ErrStream && ErrStream->IsOpen()) {
        return *ErrStream;
    }
    throw TException("No valid error fd");
}

void TSubprocess::ForkExec() {
    if ((ChildPid = fork()) < 0) {
        throw std::system_error{std::error_code{errno, std::system_category()}};
    } else if (ChildPid == 0) {
        {
            auto [inRead, inWrite] = std::move(InFds);
            if (dup2(inRead.Get(), STDIN_FILENO) < 0) {
                throw std::system_error{std::error_code{errno, std::system_category()}};
            }
        }
        {
            auto [outRead, outWrite] = std::move(OutFds);
            if (dup2(outWrite.Get(), STDOUT_FILENO) < 0) {
                throw std::system_error{std::error_code{errno, std::system_category()}};
            }
        }
        {
            auto [errRead, errWrite] = std::move(ErrFds);
            if (dup2(errWrite.Get(), STDERR_FILENO) < 0) {
                throw std::system_error{std::error_code{errno, std::system_category()}};
            }
        }

        if (execve(Executable.c_str(), PreparedArgs.data(), nullptr) < 0) {
            throw std::system_error{std::error_code{errno, std::system_category()}};
        }
    }
    InFds.first.Reset();
    OutFds.second.Reset();
    ErrFds.second.Reset();
}
