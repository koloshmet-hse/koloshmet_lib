#pragma once

#include <lib/posix/net/socket.h>

#include <unordered_map>
#include <chrono>
#include <shared_mutex>

enum class EPollEvent : unsigned char {
    IN,
    OUT,
    HUP,
    ERR
};

class TSocketPool {
public:
    class TEvent {
    public:
        explicit TEvent(short event) noexcept;

        [[nodiscard]]
        bool In() const noexcept;

        [[nodiscard]]
        bool Out() const noexcept;

        [[nodiscard]]
        bool Hup() const noexcept;

        [[nodiscard]]
        bool Err() const noexcept;

    private:
        unsigned short Event;
    };

    using TPollEvent = std::pair<TConnectedSocket&, TEvent>;

public:
    explicit TSocketPool(std::size_t capacity);

    std::vector<TPollEvent> Get(std::chrono::milliseconds timeout);

    void Add(TConnectedSocket&& socket, EPollEvent event);

    void Set(const TConnectedSocket& socket, EPollEvent event);

    void Remove(const TConnectedSocket& socket);

private:
    mutable std::shared_mutex Mutex;
    std::unordered_map<int, std::pair<TConnectedSocket, EPollEvent>> Sockets;
    std::pair<TIFdStream, TOFdStream> Pipe;
    mutable std::any PollFds;
};
