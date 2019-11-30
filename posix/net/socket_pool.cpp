#include "socket_pool.h"

#include <sys/poll.h>
#include <util/exception/exception.h>

#include <vector>

std::vector<pollfd>& AsPollFds(std::any& pollFds) {
    return std::any_cast<std::vector<pollfd>&>(pollFds);
}

TSocketPool::TSocketPool()
    : Sockets{}
    , PollFds{std::in_place_type_t<std::vector<pollfd>>{}}
{}

TSocketPool::TSocketPool(size_t capacity)
    : Sockets{}
    , PollFds{std::in_place_type_t<std::vector<pollfd>>{}}
{
    Sockets.reserve(capacity);
    AsPollFds(PollFds).reserve(capacity);
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

const TSocketPool::TPollEvent* TSocketPool::Get(std::chrono::milliseconds timeout) const {
    auto& pollFds = AsPollFds(PollFds);
    pollFds.clear();

    std::unique_lock lock{Mutex};
    for (auto&& [socket, event] : Sockets) {
        pollFds.emplace_back();
        pollFds.back().fd = socket.GetFd().Get();
        pollFds.back().events = ConvertEvent(event);
    }
    lock.unlock();

    if (int res = poll(pollFds.data(), pollFds.size(), timeout.count()); res == -1) {
        throw std::system_error{std::error_code{errno, std::system_category()}};
    } else if (res == 0) {
        return nullptr;
    } else {
        for (auto&& fd : pollFds) {

        }
    }
}

void TSocketPool::Add(TConnectedSocket&& socket, EPollEvent event) {
    std::lock_guard lock{Mutex};
    if (AsPollFds(PollFds).capacity() == Sockets.size()) {
        throw TException{"Too many sockets in pool"};
    }
    Sockets.emplace(std::move(socket), event);
}

void TSocketPool::Remove(const TConnectedSocket& event) {
    std::lock_guard lock{Mutex};
    Sockets.erase(event);
}
