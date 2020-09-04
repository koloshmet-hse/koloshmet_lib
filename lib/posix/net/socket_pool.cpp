#include "socket_pool.h"

#include <sys/poll.h>
#include <lib/util/exception/exception.h>

#include <vector>

namespace {
    std::vector<pollfd>& AsPollFds(std::any& pollFds) {
        return std::any_cast<std::vector<pollfd>&>(pollFds);
    }

    short ConvertEvent(EPollEvent event) {
        switch (event) {
            case EPollEvent::IN:
                return POLLIN;
            case EPollEvent::OUT:
                return POLLOUT;
            case EPollEvent::ERR:
                return POLLERR;
            case EPollEvent::HUP:
                return POLLHUP;
            default:
                return 0;
        }
    }
}

TSocketPool::TEvent::TEvent(short event) noexcept
    : Event(event)
{}

bool TSocketPool::TEvent::In() const noexcept {
    return (Event & static_cast<unsigned short>(POLLIN)) != 0;
}

bool TSocketPool::TEvent::Out() const noexcept {
    return (Event & static_cast<unsigned short>(POLLOUT)) != 0;
}

bool TSocketPool::TEvent::Hup() const noexcept {
    return (Event & static_cast<unsigned short>(POLLHUP)) != 0;
}

bool TSocketPool::TEvent::Err() const noexcept {
    return (Event & static_cast<unsigned short>(POLLERR)) != 0;
}

namespace {
    std::pair<TIFdStream, TOFdStream> InitPipe() {
        auto [in, out] = NInternal::Pipe();
        return {TIFdStream{std::move(in)}, TOFdStream{std::move(out)}};
    }
}

TSocketPool::TSocketPool(std::size_t capacity)
    : Mutex{}
    , Sockets{}
    , Pipe{InitPipe()}
    , PollFds{std::in_place_type_t<std::vector<pollfd>>{}}
{
    Sockets.reserve(capacity);
    AsPollFds(PollFds).reserve(capacity + 1);
}

std::vector<TSocketPool::TPollEvent> TSocketPool::Get(std::chrono::milliseconds timeout) {
    auto& pollFds = AsPollFds(PollFds);
    pollFds.clear();

    std::shared_lock lock{Mutex};
    pollFds.emplace_back();
    pollFds.back().fd = Pipe.first.GetFd().Get();
    pollFds.back().events = ConvertEvent(EPollEvent::IN);
    pollFds.back().revents = 0;
    for (auto&& [wrapper, pollEvent] : Sockets) {
        auto&& [socket, event] = pollEvent;
        pollFds.emplace_back();
        pollFds.back().fd = socket.GetFd().Get();
        pollFds.back().events = ConvertEvent(event);
        pollFds.back().revents = 0;
    }
    lock.unlock();

    std::vector<TSocketPool::TPollEvent> result;
    if (int res = poll(pollFds.data(), pollFds.size(), timeout.count()); res == -1) {
        throw std::system_error{std::error_code{errno, std::system_category()}};
    } else if (res > 0) {
        static char command;
        lock.lock();

        if (!pollFds.empty() && TEvent{pollFds.front().revents}.In()) {
            Pipe.first >> command;
            pollFds.front().revents = 0;
        }
        for (auto fd = pollFds.begin() + 1; fd != pollFds.end(); ++fd) {
            if (auto& pollEvent = Sockets.at(fd->fd); fd->revents != 0) {
                result.emplace_back(pollEvent.first, TEvent{fd->revents});
            }
        }
    }
    return result;
}

void TSocketPool::Add(TConnectedSocket&& socket, EPollEvent event) {
    std::unique_lock lock{Mutex};
    if (AsPollFds(PollFds).capacity() < Sockets.size()) {
        throw TException{"Too many sockets in pool"};
    }
    auto id = socket.GetId();
    Sockets.emplace(id, std::pair{std::move(socket), event});
    Pipe.second << '0' << std::flush;
}

void TSocketPool::Set(const TConnectedSocket& socket, EPollEvent event) {
    std::unique_lock lock{Mutex};
    Sockets.at(socket.GetId()).second = event;
}

void TSocketPool::Remove(const TConnectedSocket& event) {
    std::unique_lock lock{Mutex};
    Sockets.erase(event.GetId());
}
