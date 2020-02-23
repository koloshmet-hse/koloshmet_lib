#include "subprocess.h"

#include <unistd.h>
#include <sys/wait.h>
#include <csignal>

#include <util/exception/exception.h>

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

void TSubprocess::AddEnv(std::string var, std::string_view value) {
    var += '=';
    var += value;
    EnvVars.push_back(std::move(var));
    PreparedEnv.push_back(EnvVars.back().data());
}

void TSubprocess::ClearEnv() {
    EnvVars.clear();
    PreparedEnv.clear();
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

        if (PreparedEnv.empty()) {
            auto exec = Executable.filename() == Executable ? execvp : execv;
            if (exec(Executable.c_str(), PreparedArgs.data()) < 0) {
                throw std::system_error{std::error_code{errno, std::system_category()}};
            }
        } else {
            auto exec = Executable.filename() == Executable ? execvpe : execve;
            if (exec(Executable.c_str(), PreparedArgs.data(), PreparedEnv.data()) < 0) {
                throw std::system_error{std::error_code{errno, std::system_category()}};
            }
        }
    }
    InFds.first.Reset();
    OutFds.second.Reset();
    ErrFds.second.Reset();
}

void TSubprocess::CheckExecutable() const {
    if (!exists(Executable)) {
        throw TException{Executable, " doesn't exists"};
    }
    if (!is_regular_file(Executable)) {
        throw TException{Executable, " isn't file"};
    }

    if (access(Executable.c_str(), X_OK) < 0) {
        throw std::system_error{std::error_code{errno, std::system_category()}};
    }
}
