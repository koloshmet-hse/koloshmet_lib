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
    using TPollEvent = std::unordered_map<TConnectedSocket, EPollEvent>::iterator::value_type;

public:
    TSocketPool();

    explicit TSocketPool(size_t capacity);

    const TPollEvent* Get(std::chrono::milliseconds timeout) const;

    void Add(TConnectedSocket&& socket, EPollEvent event);

    void Remove(const TConnectedSocket& socket);

private:


private:
    mutable std::mutex Mutex;
    std::unordered_map<TConnectedSocket, EPollEvent> Sockets;
    mutable std::any PollFds;
};