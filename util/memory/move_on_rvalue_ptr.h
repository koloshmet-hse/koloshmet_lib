#pragma once

#include <memory>

template <typename T>
class TMoveOnRvaluePtr {
public:
    explicit TMoveOnRvaluePtr(const T& t)
        : Ptr{std::addressof(t)}
        , Owner{false}
    {}

    explicit TMoveOnRvaluePtr(T&& t)
        : Ptr{new T(std::move(t))}
        , Owner{true}
    {}

    ~TMoveOnRvaluePtr() {
        if (Owner) {
            delete Ptr;
        }
    }

    TMoveOnRvaluePtr(const TMoveOnRvaluePtr&) = delete;
    TMoveOnRvaluePtr& operator=(const TMoveOnRvaluePtr&) = delete;

    TMoveOnRvaluePtr(TMoveOnRvaluePtr&& ptr) noexcept
        : Ptr{ptr.Ptr}
        , Owner{ptr.Owner}
    {
        ptr.Ptr = nullptr;
        ptr.Owner = false;
    }

    TMoveOnRvaluePtr& operator=(TMoveOnRvaluePtr&& ptr) noexcept {
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
