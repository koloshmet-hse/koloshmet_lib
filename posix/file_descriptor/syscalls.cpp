#include "syscalls.h"

#include <unistd.h>
#include <fcntl.h>

#include <cstdio>

#include <util/exception/exception.h>
#include <array>

size_t NInternal::BuffSize() {
    return BUFSIZ;
}

size_t NInternal::Read(const IFd& fd, std::byte* data, std::size_t sz) {
    return read(fd.Get(), data, sz);
}

size_t NInternal::Write(const IFd& fd, const std::byte* data, std::size_t sz) {
    return write(fd.Get(), data, sz);
}

std::pair<TUniqueFd, TUniqueFd> NInternal::Pipe() {
    std::array<int, 2> fds{};
    if (pipe(fds.data()) < 0) {
        throw std::system_error{std::error_code{errno, std::system_category()}};
    }
    return {TUniqueFd{fds[0]}, TUniqueFd{fds[1]}};
}

std::pair<TSharedFd, TSharedFd> NInternal::PtMasterSlave() {
    auto master = posix_openpt(O_RDWR | O_NOCTTY);
    if (master < 0) {
        throw std::system_error{std::error_code{errno, std::system_category()}};
    }
    if (grantpt(master) < 0) {
        throw std::system_error{std::error_code{errno, std::system_category()}};
    }
    if (unlockpt(master) < 0) {
        throw std::system_error{std::error_code{errno, std::system_category()}};
    }
    const char* slaveName = ptsname(master);
    if (slaveName == nullptr) {
        throw std::system_error{std::error_code{errno, std::system_category()}};
    }
    auto slave = open(slaveName, O_RDWR);
    if (slave < 0) {
        throw std::system_error{std::error_code{errno, std::system_category()}};
    }
    return {TSharedFd{master}, TSharedFd{slave}};
}
