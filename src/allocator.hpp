#pragma once

#include <new>
#include <stddef.h>
#include <utility>

struct Allocator
{
    virtual void* Allocate(size_t size) = 0;
    virtual void Deallocate(void* ptr) = 0;
    virtual const char* GetName() const = 0;

    template<typename T, typename... Args>
    T* New(Args&&... args)
    {
        return ::new (Allocate(sizeof(T))) T(std::forward<Args>(args)...);
    }

    template<typename T>
    void Delete(T* ptr)
    {
        if (ptr) {
            ptr->~T();
            Deallocate(ptr);
        }
    }
};
