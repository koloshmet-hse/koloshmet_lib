#include <posix/file_descriptor/shared_fd.h>

namespace NInternal {
    std::size_t BuffSize();

    std::size_t Read(const IFd& fd, std::byte* data, std::size_t sz);

    std::size_t Write(const IFd& fd, const std::byte* data, std::size_t sz);

    std::pair<TUniqueFd, TUniqueFd> Pipe();

    std::pair<TSharedFd, TSharedFd> PtMasterSlave();
}
