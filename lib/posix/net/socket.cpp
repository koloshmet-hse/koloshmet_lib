#include "socket.h"

#include <sys/socket.h>

#include <lib/util/exception/exception.h>

#include <system_error>
#include <sys/un.h>
#include <netinet/in.h>

ESocket TSocketAddress::GetType() const noexcept {
    return Type;
}

TSocketAddress::TSocketAddress(TSocketAddress&& address) noexcept
    : SocketAddress{std::move(address.SocketAddress)}
    , Type{address.Type}
{}

TSocketAddress& TSocketAddress::operator=(TSocketAddress&& address) noexcept {
    SocketAddress = std::move(address.SocketAddress);
    Type = address.Type;
    return *this;
}

namespace {
    TUniqueFd AcquireTCPSocket(ESocket socketType) {
        int type;
        switch (socketType) {
            case ESocket::IP:
                type = PF_INET;
                break;
            case ESocket::UNIX:
                type = PF_LOCAL;
                break;
            default:
                throw TException{"Wrong socket type"};
        }

        if (int fd = socket(type, SOCK_STREAM, 0); fd != -1) {
            return TUniqueFd{fd};
        }
        throw std::system_error{std::error_code{errno, std::system_category()}};
    }
}

TSocket::TSocket(ESocket socketType)
    : TUniqueFd{AcquireTCPSocket(socketType)}
    , TSocketAddress{socketType}
{
    switch (socketType) {
        case ESocket::IP:
            SocketAddress.emplace<sockaddr_in>();
            break;
        case ESocket::UNIX:
            SocketAddress.emplace<sockaddr_un>();
            break;
        default:
            throw TException{"Wrong socket type"};
    }
}

TConnectedSocket::TConnectedSocket(TUniqueFd&& fd, const TSocketAddress& sockAddr)
    : TFdStream{std::move(fd)}
    , TSocketAddress{sockAddr}
    , Id{GetFd().Get()}
{}

TConnectedSocket::TConnectedSocket(TUniqueFd&& fd, TSocketAddress&& sockAddr)
    : TFdStream{std::move(fd)}
    , TSocketAddress{std::move(sockAddr)}
    , Id{GetFd().Get()}
{}

TConnectedSocket::TConnectedSocket(TConnectedSocket&& other) noexcept
    : TFdStream{std::move(dynamic_cast<TFdStream&>(other))}
    , TSocketAddress{std::move(dynamic_cast<TSocketAddress&>(other))}
    , Id{0}
{
    std::swap(Id, other.Id);
}

TConnectedSocket::~TConnectedSocket() {
    if (Id != 0) {
        shutdown(Id, SHUT_RDWR);
    }
}

TConnectedSocket& TConnectedSocket::operator=(TConnectedSocket&& other) noexcept {
    dynamic_cast<TFdStream&>(*this) = std::move(dynamic_cast<TFdStream&>(other));
    dynamic_cast<TSocketAddress&>(*this) = std::move(dynamic_cast<TSocketAddress&>(other));
    auto tmpId = other.Id;
    other.Id = 0;
    Id = tmpId;
    return *this;
}

int TConnectedSocket::GetId() const noexcept {
    return Id;
}

namespace {
    void ReconnectImpl(TConnectedSocket& socket, bool shut = true) {
        sockaddr* addrPtr;
        std::size_t addrLen;
        switch (socket.GetType()) {
            case ESocket::IP:
                addrPtr = reinterpret_cast<sockaddr*>(socket.AddressPtr<sockaddr_in>());
                addrLen = sizeof(socket.Address<sockaddr_in>());
                break;
            case ESocket::UNIX:
                addrPtr = reinterpret_cast<sockaddr*>(socket.AddressPtr<sockaddr_un>());
                addrLen = sizeof(socket.Address<sockaddr_un>());
                break;
            default:
                throw TException{"Wrong socket type"};
        }
        if (shut) {
            if (shutdown(socket.GetFd().Get(), SHUT_RDWR) < 0) {
                throw std::system_error{std::error_code{errno, std::system_category()}};
            }
        }
        if (connect(socket.GetFd().Get(), addrPtr, addrLen) < 0) {
            throw std::system_error{std::error_code{errno, std::system_category()}};
        }
    }
}

TConnectedSocket& Reconnect(TConnectedSocket& socket) {
    ReconnectImpl(socket);
    return socket;
}

TConnectedSocket Connect(TSocket&& socket) {
    TConnectedSocket connected{
        std::move(dynamic_cast<TUniqueFd&>(socket)),
        std::move(dynamic_cast<TSocketAddress&>(socket))
    };
    ReconnectImpl(connected, false);
    return connected;
}
