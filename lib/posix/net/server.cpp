#include "server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>

#include <util/exception/exception.h>

#include <cstring>

namespace {
    template <typename TSocketAddress>
    void Bind(TSocket& socket, TSocketAddress& socketAddress) {
        auto addrPtr = reinterpret_cast<sockaddr*>(std::addressof(socketAddress));
        if (bind(socket.Get(), addrPtr, sizeof(socketAddress)) < 0) {
            throw std::system_error{std::error_code{errno, std::system_category()}};
        }
    }

    void Listen(TSocket& socket, std::size_t connects) {
        if (listen(socket.Get(), connects) < 0) {
            throw std::system_error{std::error_code{errno, std::system_category()}};
        }
    }
}

void NInternal::InitIPSocket(TSocket& socket, int port, int connects) {
    auto& sockAddr = socket.Address<sockaddr_in>();
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);
    sockAddr.sin_addr.s_addr = INADDR_ANY;

    Bind(socket, sockAddr);
    Listen(socket, connects);
}

void NInternal::InitUNIXSocket(TSocket& socket, const std::filesystem::path& socketPath, int connects) {
    auto& sockAddr = socket.Address<sockaddr_un>();
    sockAddr.sun_family = AF_LOCAL;
    std::strncpy(sockAddr.sun_path, socketPath.c_str(), sizeof(sockAddr.sun_path) - 1);

    Bind(socket, sockAddr);
    Listen(socket, connects);
}

void NInternal::ReleaseUnixAddress(TSocket& socket) {
    auto& sockAddr = socket.Address<sockaddr_un>();
    std::filesystem::path unixAddress = sockAddr.sun_path;
    remove(unixAddress);
}

TConnectedSocket NInternal::Accept(const TSocket& socket) {
    int fd;
    TSocketAddress sockAddr{socket.GetType()};
    switch (socket.GetType()) {
        case ESocket::IP: {
            sockAddr.Emplace<sockaddr_in>();
            socklen_t sockLen = sizeof(sockaddr_in);
            fd = accept(
                socket.Get(),
                reinterpret_cast<sockaddr*>(sockAddr.AddressPtr<sockaddr_in>()),
                std::addressof(sockLen));
            break;
        }
        case ESocket::UNIX: {
            sockAddr.Emplace<sockaddr_un>();
            socklen_t sockLen = sizeof(sockaddr_un);
            fd = accept(
                socket.Get(),
                reinterpret_cast<sockaddr*>(sockAddr.AddressPtr<sockaddr_un>()),
                std::addressof(sockLen));
            break;
        }
        default:
            throw TException{"Unknown socket type in accept"};
    }

    if (fd < 0) {
        throw std::system_error{std::error_code{errno, std::system_category()}};
    }
    return {TUniqueFd{fd}, std::move(sockAddr)};
}
