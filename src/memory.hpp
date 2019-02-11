#ifndef BLOCKS_MEMORY_HPP
#define BLOCKS_MEMORY_HPP

#include <stddef.h>
#include <string.h>
#include <assert.h>

struct Memory
{
    void* ptr;
    size_t size;

    static Memory New(size_t size)
    {
        Memory mem = {};
        mem.ptr = calloc(1, size);
        assert(mem.ptr);
        mem.size = size;
        return mem;
    }

    inline void
    static Delete(Memory mem)
    {
        free(mem.ptr);
    }
};

#endif // BLOCKS_MEMORY_HPP
