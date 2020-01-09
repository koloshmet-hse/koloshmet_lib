#include "server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>

#include <util/exception/exception.h>

#include <cstring>

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

TServer::TServer(int port, int connects)
    : Socket{ESocket::IP}
    , Running{false}
{
    auto& sockAddr = Socket.Address<sockaddr_in>();
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);
    sockAddr.sin_addr.s_addr = INADDR_ANY;

    Bind(Socket, sockAddr);
    Listen(Socket, connects);
}

TServer::TServer(const std::filesystem::path& socketPath, int connects)
    : Socket{ESocket::UNIX}
    , Running{false}
{
    auto& sockAddr = Socket.Address<sockaddr_un>();
    sockAddr.sun_family = AF_LOCAL;
    std::strncpy(sockAddr.sun_path, socketPath.c_str(), sizeof(sockAddr.sun_path) - 1);

    Bind(Socket, sockAddr);
    Listen(Socket, connects);
}

void TServer::Start(IReplier& replier) {
    Running = true;
    this->Run(replier);
}

void TServer::Stop() {
    Running = false;
}

void TServer::Run(IReplier& replier) {
    while (Running) {
        replier.Connect(Accept());
    }
}

TConnectedSocket TServer::Accept() {
    int fd;
    TSocketAddress sockAddr{Socket.GetType()};
    switch (Socket.GetType()) {
        case ESocket::IP: {
            sockAddr.Emplace<sockaddr_in>();
            socklen_t sockLen = sizeof(sockaddr_in);
            fd = accept(
                Socket.Get(),
                reinterpret_cast<sockaddr*>(sockAddr.AddressPtr<sockaddr_in>()),
                std::addressof(sockLen));
            break;
        }
        case ESocket::UNIX: {
            sockAddr.Emplace<sockaddr_un>();
            socklen_t sockLen = sizeof(sockaddr_un);
            fd = accept(
                Socket.Get(),
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
