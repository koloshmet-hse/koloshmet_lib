#pragma once

#include <memory>

template <typename T>
class TCopyOnMoveRef {
public:
    explicit TCopyOnMoveRef(const T& t)
        : Ptr{std::addressof(t)}
        , Owner{false}
    {}

    explicit TCopyOnMoveRef(T&& t)
        : Ptr{new T(std::move(t))}
        , Owner{true}
    {}

    ~TCopyOnMoveRef() {
        if (Owner) {
            delete Ptr;
        }
    }

    TCopyOnMoveRef(const TCopyOnMoveRef&) = delete;
    TCopyOnMoveRef& operator=(const TCopyOnMoveRef&) = delete;

    TCopyOnMoveRef(TCopyOnMoveRef&& ptr) noexcept
        : Ptr{ptr.Ptr}
        , Owner{ptr.Owner}
    {
        ptr.Ptr = nullptr;
        ptr.Owner = false;
    }

    TCopyOnMoveRef& operator=(TCopyOnMoveRef&& ptr) noexcept {
        Ptr = ptr.Ptr;
        Owner = ptr.Owner;
        ptr.Ptr = nullptr;
        ptr.Owner = false;
        return *this;
    }

    const T& operator*() const {
        return *Ptr;
    }

    const T* operator->() const {
        return Ptr;
    }

private:
    const T* Ptr;
    bool Owner;
};
