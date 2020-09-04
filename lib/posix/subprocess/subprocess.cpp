#include "subprocess.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <csignal>

#include <lib/util/string/utils.h>

#include <lib/util/exception/exception.h>

TSubprocess::TSubprocess(TSubprocess&& other) noexcept
    : Executable(std::move(other.Executable))
    , Arguments(std::move(other.Arguments))
    , PreparedArgs(std::move(other.PreparedArgs))
    , InStream(std::move(other.InStream))
    , OutStream(std::move(other.OutStream))
    , ErrStream(std::move(other.ErrStream))
    , ChildPid(-1)
{
    std::swap(ChildPid, other.ChildPid);
}

TSubprocess& TSubprocess::operator=(TSubprocess&& other) noexcept {
    Kill();
    Executable = std::move(other.Executable);
    Arguments = std::move(other.Arguments);
    PreparedArgs = std::move(other.PreparedArgs);
    InStream = std::move(other.InStream);
    OutStream = std::move(other.OutStream);
    ErrStream = std::move(other.ErrStream);
    auto tmp = other.ChildPid;
    other.ChildPid = -1;
    ChildPid = tmp;
    return *this;
}

TSubprocess::~TSubprocess() {
    try {
        Kill();
        Wait();
    } catch (...) {
        // destructor must be noexcept
    }
}

TSubprocess::EExitCode TSubprocess::Wait() {
    if (ChildPid > 0) {
        int status;
        if (waitpid(ChildPid, std::addressof(status), 0) < 0) {
            throw std::system_error{std::error_code{errno, std::system_category()}};
        }
        if (WIFEXITED(status)) {
            return EExitCode{WEXITSTATUS(status)};
        } else if (WIFSIGNALED(status)) {
            return EExitCode::Signaled;
        } else {
            return EExitCode::Unknown;
        }
    } else {
        throw TException{"The process has not been run"};
    }
    return EExitCode::Unknown;
}

void TSubprocess::Kill() {
    if (ChildPid > 0) {
        if (kill(ChildPid, SIGKILL) < 0) {
            throw std::system_error{std::error_code{errno, std::system_category()}};
        }
    }
    ChildPid = -1;
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

void TSubprocess::ForkExec(ECommunicationMode mode) {
    std::pair<TSharedFd, TSharedFd> inFds;
    std::pair<TSharedFd, TSharedFd> outFds;
    std::pair<TSharedFd, TSharedFd> errFds;
    {
        std::pair<TSharedFd, TSharedFd> ptFds;
        if (mode & ECommunicationMode::Pt) {
            ptFds = NInternal::PtMasterSlave();
        }

        if (mode & ECommunicationMode::In) {
            if (mode & ECommunicationMode::InPt) {
                inFds = ptFds;
                std::swap(inFds.first, inFds.second);
            } else {
                inFds = NInternal::Pipe();
            }
        }
        if (mode & ECommunicationMode::Out) {
            if (mode & ECommunicationMode::OutPt) {
                outFds = ptFds;
            } else {
                outFds = NInternal::Pipe();
            }
        }
        if (mode & ECommunicationMode::Err) {
            if (mode & ECommunicationMode::ErrPt) {
                errFds = ptFds;
            } else {
                errFds = NInternal::Pipe();
            }
        }
    }

    if ((ChildPid = fork()) < 0) {
        throw std::system_error{std::error_code{errno, std::system_category()}};
    } else if (ChildPid == 0) {
        setsid();
        if (ioctl(inFds.first.Get(), TIOCSCTTY, nullptr) < 0) {
            throw std::system_error{std::error_code{errno, std::system_category()}};
        }
        {
            auto [inRead, inWrite] = std::move(inFds);
            if (mode & ECommunicationMode::In && dup2(inRead.Get(), STDIN_FILENO) < 0) {
                throw std::system_error{std::error_code{errno, std::system_category()}};
            }
        }
        {
            auto [outRead, outWrite] = std::move(outFds);
            if (mode & ECommunicationMode::Out && dup2(outWrite.Get(), STDOUT_FILENO) < 0) {
                throw std::system_error{std::error_code{errno, std::system_category()}};
            }
        }
        {
            auto [errRead, errWrite] = std::move(errFds);
            if (mode & ECommunicationMode::Err && dup2(errWrite.Get(), STDERR_FILENO) < 0) {
                throw std::system_error{std::error_code{errno, std::system_category()}};
            }
        }

        if (PreparedEnv.empty()) {
            if (execv(Executable.c_str(), PreparedArgs.data()) < 0) {
                throw std::system_error{std::error_code{errno, std::system_category()}};
            }
        } else {
            if (execve(Executable.c_str(), PreparedArgs.data(), PreparedEnv.data()) < 0) {
                throw std::system_error{std::error_code{errno, std::system_category()}};
            }
        }
    } else {
        auto [inRead, inWrite] = std::move(inFds);
        auto [outRead, outWrite] = std::move(outFds);
        auto [errRead, errWrite] = std::move(errFds);
        if (inWrite) {
            InStream.emplace(std::move(inWrite));
        }
        if (outRead) {
            OutStream.emplace(std::move(outRead));
        }
        if (errRead) {
            ErrStream.emplace(std::move(errRead));
        }
    }
}

void TSubprocess::PrepareArgs() {
    for (auto&& cur : Arguments) {
        PreparedArgs.push_back(cur.data());
    }
    PreparedArgs.push_back(nullptr);
}

void TSubprocess::PrepareEnvs() {
    for (auto&& cur : EnvVars) {
        PreparedEnv.push_back(cur.data());
    }
    PreparedEnv.push_back(nullptr);
}

std::filesystem::path TSubprocess::FindExecutablePath(std::filesystem::path executable) {
    if (executable != executable.filename()) {
        if (!exists(executable)) {
            throw TException{executable, " file doesn't exist"};
        }
        if (is_symlink(executable)) {
            executable = read_symlink(executable);
            if (!exists(executable)) {
                throw TException{executable, " file doesn't exist"};
            }
        }
        if (!is_regular_file(executable)) {
            throw TException{executable, " file isn't regular"};
        }
        if (access(executable.c_str(), X_OK) != 0) {
            throw TException{executable, " file can't be executed"};
        }
        return executable;
    }
    std::string_view path = std::getenv("PATH");
    for (std::filesystem::path curPath : Split(path, ":")) {
        if (exists(curPath) && is_symlink(curPath)) {
            curPath = read_symlink(curPath);
        }
        if (exists(curPath) && is_directory(curPath)) {
            for (std::filesystem::path file : std::filesystem::directory_iterator{curPath}) {
                if (file.has_filename() && file.filename() == executable.filename()) {
                    if (exists(file) && is_symlink(file)) {
                        file = read_symlink(file);
                    }
                    if (exists(file) && is_regular_file(file) && access(file.c_str(), X_OK) == 0) {
                        return file;
                    }
                }
            }
        }
    }
    throw TException{"Can't find ", executable, " in $PATH"};
}

bool operator&(TSubprocess::ECommunicationMode l, TSubprocess::ECommunicationMode r) {
    return (static_cast<std::uint_least32_t>(l) & static_cast<std::uint_least32_t>(r)) != 0;
}

TSubprocess::ECommunicationMode operator|(TSubprocess::ECommunicationMode l, TSubprocess::ECommunicationMode r) {
    return static_cast<TSubprocess::ECommunicationMode>(
        static_cast<std::uint_least32_t>(l) | static_cast<std::uint_least32_t>(r));
}

