#pragma once

#include <lib/posix/net/socket.h>
#include <lib/async/stop_token/stop_token.h>

namespace NInternal {
    TConnectedSocket ConnectIP(const std::string& address, int port);

    TConnectedSocket ConnectUNIX(const std::filesystem::path& socketPath);
}

template <typename TSender>
class TClient {
public:
    TClient(TSender sender, const std::string& address, int port)
        : Socket{NInternal::ConnectIP(address, port)}
        , Sender(std::move(sender))
    {}

    TClient(TSender sender, const std::filesystem::path& socketPath)
        : Socket{NInternal::ConnectUNIX(socketPath)}
        , Sender(std::move(sender))
    {}

    virtual ~TClient() = default;

    virtual void operator()() {
        while (Sender(Socket));
    }

    virtual void operator()(TStopToken& token) {
        while (!token && Sender(Socket));
    }

private:
    TConnectedSocket Socket;
    TSender Sender;
};
