#include "fd.h"

#include <unistd.h>

#include <system_error>

void TFdCloser::Close(int fd) {
    if (close(fd) != 0) {
        throw std::system_error{std::error_code{errno, std::system_category()}};
    }
}
