#pragma once

#include <lib/posix/net/socket.h>
#include <lib/async/stop_token/stop_token.h>

#include <filesystem>

#include <any>
#include <atomic>

namespace NInternal {
    TConnectedSocket Accept(const TSocket& socket);

    void InitIPSocket(TSocket& socket, int port, int connects);

    void InitUNIXSocket(TSocket& socket, const std::filesystem::path& socketPath, int connects);

    void ReleaseUnixAddress(TSocket& socket);
}

template <typename TReplier>
class TServer {
    static_assert(std::is_invocable_r_v<bool, TReplier, TConnectedSocket>);

public:
    TServer(TReplier replier, int port, int connects)
        : Socket{ESocket::IP}
        , Replier(std::move(replier))
    {
        NInternal::InitIPSocket(Socket, port, connects);
    }

    TServer(TReplier replier, const std::filesystem::path& socketPath, int connects)
        : Socket{ESocket::UNIX}
        , Replier(std::move(replier))
    {
        NInternal::InitUNIXSocket(Socket, socketPath, connects);
    }

    virtual ~TServer() {
        if (Socket.GetType() == ESocket::UNIX) {
            NInternal::ReleaseUnixAddress(Socket);
        }
    }

public:
    virtual void operator()() {
        while (Replier(NInternal::Accept(Socket)));
    }

    virtual void operator()(TStopToken& token) {
        while (!token && Replier(NInternal::Accept(Socket)));
    }

private:
    TSocket Socket;
    TReplier Replier;
};
