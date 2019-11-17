#pragma once

#include <posix/net/socket.h>

#include <filesystem>

#include <any>

class TServer {
public:
    TServer(int port, int connects);

    TServer(const std::filesystem::path& socketPath, int connects);

    TConnectedSocket Accept();

private:
    TSocket Socket;
};
