#pragma once

#include <posix/net/replier.h>

#include <filesystem>

#include <any>

class TServer {
public:
    TServer(int port, int connects);

    TServer(const std::filesystem::path& socketPath, int connects);

    virtual ~TServer() = default;

    void Start(IReplier& replier);

    void Stop();

protected:
    virtual void Run(IReplier& replier);

    TConnectedSocket Accept();

private:
    TSocket Socket;

protected:
    std::atomic_bool Running;
};
