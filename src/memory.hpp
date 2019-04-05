#pragma once

#include <assert.h>
#include <stddef.h>
#include <string.h>

struct Memory
{
    void* ptr;
    size_t size;

    Memory(size_t size)
        : ptr(nullptr)
        , size(size)
    {}

    void Create()
    {
        ptr = calloc(1, size);
        assert(ptr);
    }

    void Destroy()
    {
        free(ptr);
        ptr = nullptr;
        size = 0;
    }
};
