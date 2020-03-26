#pragma once

#include <posix/file_descriptor/unique_fd.h>

template <typename TCloser>
class TBasicSharedFd : public IFd {
public:
    explicit TBasicSharedFd(int fd) noexcept
        : Fd{std::make_shared<TBasicUniqueFd<TCloser>>(fd)}
    {}

    /* implicit */ TBasicSharedFd(TNullFd = NULL_FD) noexcept
        : Fd{nullptr}
    {}

    /* implicit */ TBasicSharedFd(TBasicUniqueFd<TCloser>&& fd)
        : Fd{fd ? std::make_shared<TBasicUniqueFd<TCloser>>(std::move(fd)) : nullptr}
    {}

    TBasicSharedFd(const TBasicSharedFd& fd)
        : Fd{fd.Fd}
    {}

    TBasicSharedFd(TBasicSharedFd&& fd) noexcept
        : Fd{std::move(fd.Fd)}
    {}

    TBasicSharedFd& operator=(TBasicSharedFd&& fd) noexcept {
        Fd = std::move(fd.Fd);
        return *this;
    }

    TBasicSharedFd& operator=(const TBasicSharedFd& fd) {
        Fd = fd.Fd;
        return *this;
    }

    ~TBasicSharedFd() override = default;

    [[nodiscard]]
    int Get() const override {
        if (!Fd) {
            return -1;
        }
        return Fd->Get();
    }

    void Reset(int fd = -1) {
        if (fd == -1) {
            Fd = nullptr;
        } else {
            Fd = std::make_shared<TBasicUniqueFd<TCloser>>(fd);
        }
    }

    explicit operator bool() const {
        return static_cast<bool>(Fd);
    }

private:
    std::shared_ptr<TBasicUniqueFd<TCloser>> Fd;
};

using TSharedFd = TBasicSharedFd<TFdCloser>;
