#pragma once

#include <posix/file_descriptor/fd_stream.h>

#include <any>
#include <filesystem>

enum class ESocket : unsigned char {
    IP,
    UNIX
};

class TSocketAddress {
public:
    template <typename... TArgs>
    explicit TSocketAddress(ESocket type, TArgs&&... args)
        : SocketAddress{std::forward<TArgs>(args)...}
        , Type{type}
    {}

    virtual ~TSocketAddress() = default;

    TSocketAddress(const TSocketAddress& address) = default;

    TSocketAddress(TSocketAddress&& address) noexcept;

    TSocketAddress& operator=(const TSocketAddress& address) = default;

    TSocketAddress& operator=(TSocketAddress&& address) noexcept;

    template <typename TSockAddr>
    TSockAddr& Address() {
        return std::any_cast<TSockAddr&>(SocketAddress);
    }

    template <typename TSockAddr>
    TSockAddr* AddressPtr() {
        return std::addressof(std::any_cast<TSockAddr&>(SocketAddress));
    }

    template <typename TSockAddr>
    const TSockAddr& Address() const {
        return std::any_cast<const TSockAddr&>(SocketAddress);
    }

    template <typename TSockAddr>
    const TSockAddr* AddressPtr() const {
        return std::addressof(std::any_cast<const TSockAddr&>(SocketAddress));
    }

    template <typename TSockAddr, typename... TArgs>
    void Emplace(TArgs&&... args) {
        SocketAddress.emplace<TSockAddr>(std::forward<TArgs>(args)...);
    }

    [[nodiscard]]
    ESocket GetType() const noexcept;

protected:
    std::any SocketAddress;

private:
    ESocket Type;
};

class TSocket : public TUniqueFd, public TSocketAddress {
public:
    explicit TSocket(ESocket socketType);
};

class TConnectedSocket : public TFdStream, public TSocketAddress {
public:
    TConnectedSocket(TUniqueFd&& fd, const TSocketAddress& sockAddr);

    TConnectedSocket(TUniqueFd&& fd, TSocketAddress&& sockAddr);

    TConnectedSocket(TConnectedSocket&& other) noexcept;

    ~TConnectedSocket() override;

    TConnectedSocket& operator=(TConnectedSocket&& other) noexcept;

    [[nodiscard]]
    int GetId() const noexcept;

private:
    int Id;
};

TConnectedSocket Connect(TSocket&& socket);

TConnectedSocket& Reconnect(TConnectedSocket& socket);
