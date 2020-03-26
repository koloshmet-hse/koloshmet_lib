#pragma once

struct TNullFd {};

constexpr TNullFd NULL_FD{};

class IFd {
public:
    [[nodiscard]]
    virtual int Get() const = 0;

    virtual ~IFd() = default;
};

struct TFdCloser {
    static void Close(int fd);
};
