#pragma once

#include <posix/net/socket.h>

#include <unordered_map>
#include <chrono>

enum class EPollEvent : unsigned char {
    IN,
    OUT,
    HUP,
    ERR
};

class TSocketPool {
public:
    using TPollEvent = std::pair<TConnectedSocket, EPollEvent>;

public:
    TSocketPool();

    explicit TSocketPool(std::size_t capacity);

    TPollEvent* Get(std::chrono::milliseconds timeout);

    void Add(TConnectedSocket&& socket, EPollEvent event);

    void Remove(const TConnectedSocket& socket);

private:
    mutable std::mutex Mutex;
    std::unordered_map<NInternal::TConnectedSocketHashWrapper, TPollEvent> Sockets;
    mutable std::any PollFds;
};
