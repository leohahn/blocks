#pragma once

#include <assert.h>
#include <stddef.h>
#include <string.h>

struct Memory
{
    void* ptr;
    size_t size;

    Memory()
        : Memory(0)
    {}

    Memory(size_t size)
        : ptr(nullptr)
        , size(size)
    {
        if (size > 0) {
            ptr = calloc(1, size);
            assert(ptr);
        }
    }

    Memory(Memory&& other)
    {
        *this = std::move(other);
    }

    Memory& operator=(Memory&& other)
    {
        ptr = other.ptr;
        size = other.size;
        other.ptr = nullptr;
        other.size = 0;
        return *this;
    }

    ~Memory()
    {
        free(ptr);
        ptr = nullptr;
        size = 0;
    }
};
