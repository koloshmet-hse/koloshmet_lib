#include "fd.h"

#include <unistd.h>

#include <util/exception/exception.h>

void TFdCloser::Close(int fd) {
    if (close(fd) != 0) {
        throw TException{"Failure while closing fd No", fd};
    }
}
