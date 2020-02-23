#pragma once

#include "fd.h"

#include <functional>

template <typename TCloser>
class TBasicUniqueFd : public IFd {
public:
    explicit TBasicUniqueFd(int fd) noexcept;

    /* implicit */ TBasicUniqueFd(TNullFd fd = NullFd) noexcept;

    TBasicUniqueFd(TBasicUniqueFd&& fd) noexcept;

    TBasicUniqueFd& operator=(TBasicUniqueFd&& fd) noexcept;

    ~TBasicUniqueFd() override;

    [[nodiscard]]
    int Get() const noexcept override;

    [[nodiscard]]
    int Release() noexcept;

    void Reset(int fd = -1);

    explicit operator bool() const {
        return Fd != -1;
    }

private:
    void Close();

private:
    int Fd;
};


template <typename TCloser>
TBasicUniqueFd<TCloser>::TBasicUniqueFd(int fd) noexcept
    : Fd{fd}
{}

template <typename TCloser>
TBasicUniqueFd<TCloser>::TBasicUniqueFd(TNullFd) noexcept
    : Fd{-1}
{}

template <typename TCloser>
TBasicUniqueFd<TCloser>::TBasicUniqueFd(TBasicUniqueFd&& fd) noexcept
    : Fd{fd.Fd}
{
    fd.Fd = -1;
}

template <typename TCloser>
TBasicUniqueFd<TCloser>& TBasicUniqueFd<TCloser>::operator=(TBasicUniqueFd&& fd) noexcept {
    Close();
    Fd = fd.Fd;
    fd.Fd = -1;
    return *this;
}

template <typename TCloser>
TBasicUniqueFd<TCloser>::~TBasicUniqueFd() {
    try {
        Close();
    } catch (...) {
        // destructor must be noexcept
    }

}

template <typename TCloser>
int TBasicUniqueFd<TCloser>::Get() const noexcept {
    return Fd;
}

template <typename TCloser>
int TBasicUniqueFd<TCloser>::Release() noexcept {
    auto fd = Fd;
    Fd = -1;
    return fd;
}

template <typename TCloser>
void TBasicUniqueFd<TCloser>::Reset(int fd) {
    Close();
    Fd = fd;
}

template <typename TCloser>
void TBasicUniqueFd<TCloser>::Close() {
    if (Fd > 0) {
        TCloser::Close(Fd);
    }
}

using TUniqueFd = TBasicUniqueFd<TFdCloser>;

template <typename TCloser = TFdCloser, typename TSysCall, typename... TArgs>
TBasicUniqueFd<TCloser> MakeUniqueFd(TSysCall&& sysCall, TArgs&&... args) {
    return TBasicUniqueFd{std::invoke(std::forward<TSysCall>(sysCall), std::forward<TArgs>(args)...)};
}
