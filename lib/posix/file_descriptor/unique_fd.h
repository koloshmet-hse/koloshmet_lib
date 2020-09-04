#pragma once

#include "fd.h"

#include <functional>

template <typename TCloser>
class TBasicUniqueFd : public IFd {
public:
    explicit TBasicUniqueFd(int fd) noexcept
        : Fd{fd}
    {}

    /* implicit */ TBasicUniqueFd(TNullFd = NULL_FD) noexcept
        : Fd{-1}
    {}

    TBasicUniqueFd(TBasicUniqueFd&& fd) noexcept
        : Fd{-1}
    {
        std::swap(Fd, fd.Fd);
    }

    TBasicUniqueFd& operator=(TBasicUniqueFd&& fd) noexcept {
        if (fd.Fd == Fd) {
            return *this;
        }
        Close();
        Fd = fd.Fd;
        fd.Fd = -1;
        return *this;
    }

    ~TBasicUniqueFd() override {
        try {
            Close();
        } catch (...) {
            // destructor must be noexcept
        }
    }

    [[nodiscard]]
    int Get() const override {
        return Fd;
    }

    [[nodiscard]]
    int Release() noexcept {
        auto fd = Fd;
        Fd = -1;
        return fd;
    }

    void Reset(int fd = -1) {
        Close();
        Fd = fd;
    }

    explicit operator bool() const {
        return Fd != -1;
    }

private:
    void Close() {
        if (Fd > 0) {
            TCloser::Close(Fd);
        }
    }

private:
    int Fd;
};

using TUniqueFd = TBasicUniqueFd<TFdCloser>;
