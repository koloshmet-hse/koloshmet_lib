#pragma once

struct TNullFd {};

constexpr TNullFd NullFd{};

class IFd {
public:
    [[nodiscard]]
    virtual int Get() const noexcept = 0;

    virtual ~IFd() = default;
};

struct TFdCloser {
    static void Close(int fd);
};
