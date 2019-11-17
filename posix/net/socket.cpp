#include "socket.h"

#include <sys/socket.h>

#include <util/exception/exception.h>

#include <system_error>
#include <sys/un.h>
#include <netinet/in.h>

ESocket TSocketAddress::GetType() const {
    return Type;
}

TUniqueFd AcquireTCPSocket(ESocket socketType) {
    int type;
    switch (socketType) {
        case ESocket::IP:
            type = PF_INET;
            break;
        case ESocket::UNIX:
            type = PF_UNIX;
            break;
        default:
            throw TException{"Wrong socket type"};
    }

    if (int fd = socket(type, SOCK_STREAM, 0); fd != -1) {
        return TUniqueFd{fd};
    }
    throw std::system_error{std::error_code{errno, std::system_category()}};
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
{}

TConnectedSocket::TConnectedSocket(TUniqueFd&& fd, TSocketAddress&& sockAddr)
    : TFdStream{std::move(fd)}
    , TSocketAddress{std::move(sockAddr)}
{}
