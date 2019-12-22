#pragma once

#include <memory>

template <typename T, typename TAllocator = std::allocator<T>>
class TMoveOnRvaluePtr {
public:
    explicit TMoveOnRvaluePtr(const T& t)
        : Allocator{}
        , Ptr{std::addressof(t)}
        , Owner{false}
    {}

    explicit TMoveOnRvaluePtr(T&& t, const TAllocator& allocator = TAllocator{})
        : Allocator{allocator}
        , Ptr{std::allocator_traits<TAllocator>::allocate(Allocator, 1)}
        , Owner{true}
    {
        std::allocator_traits<TAllocator>::construct(Allocator, Ptr, std::move(t));
    }

    ~TMoveOnRvaluePtr() {
        if (Owner) {
            std::allocator_traits<TAllocator>::destroy(Allocator, Ptr);
            std::allocator_traits<TAllocator>::deallocate(Allocator, Ptr, 1);
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
        if (ptr.Ptr == Ptr) {
            return *this;
        }
        if (Owner) {
            std::allocator_traits<TAllocator>::destroy(Allocator, Ptr);
            std::allocator_traits<TAllocator>::deallocate(Allocator, Ptr, 1);
        }
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
    TAllocator Allocator;
    T* Ptr;
    bool Owner;
};
