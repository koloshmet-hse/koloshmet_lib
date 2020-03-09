#pragma once

#include <memory>

template <typename T, typename TAllocator = std::allocator<T>>
class TMoveOnRvaluePtr : private TAllocator {
    static_assert(std::is_same_v<T, std::remove_cv_t<std::remove_reference_t<T>>>);

public:
    explicit TMoveOnRvaluePtr(const T& t)
        : TAllocator{}
        , Ptr{std::addressof(t)}
        , Owner{false}
    {}

    explicit TMoveOnRvaluePtr(T&& t, TAllocator allocator = TAllocator{})
        : TAllocator{std::move(allocator)}
        , Ptr{std::allocator_traits<TAllocator>::allocate(*this, 1)}
        , Owner{true}
    {
        try {
            std::allocator_traits<TAllocator>::construct(*this, const_cast<T*>(Ptr), std::move(t));
        } catch (...) {
            std::allocator_traits<TAllocator>::deallocate(*this, const_cast<T*>(Ptr), 1);
            throw;
        }
    }

    ~TMoveOnRvaluePtr() {
        if (Owner) {
            std::allocator_traits<TAllocator>::destroy(*this, const_cast<T*>(Ptr));
            std::allocator_traits<TAllocator>::deallocate(*this, const_cast<T*>(Ptr), 1);
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
            std::allocator_traits<TAllocator>::destroy(*this, const_cast<T*>(Ptr));
            std::allocator_traits<TAllocator>::deallocate(*this, const_cast<T*>(Ptr), 1);
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

    const T* Get() const {
        return Ptr;
    }

private:
    const T* Ptr;
    bool Owner;
};
