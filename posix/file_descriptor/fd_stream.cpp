#include "fd_stream.h"

#include <unistd.h>

#include <cstdio>

size_t NInternal::BuffSize() {
    return BUFSIZ;
}

size_t NInternal::Read(const IFd& fd, std::byte* data, std::size_t sz) {
    return read(fd.Get(), data, sz);
}

size_t NInternal::Write(const IFd& fd, const std::byte* data, std::size_t sz) {
    return write(fd.Get(), data, sz);
}
