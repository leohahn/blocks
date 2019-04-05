#pragma once

#include "Allocator.hpp"
#include "Defines.hpp"
#include "Memory.hpp"

class LinearAllocator : public Allocator
{
public:
    LinearAllocator(Memory mem)
        : LinearAllocator(mem.ptr, mem.size)
    {}

    LinearAllocator(Memory mem, size_t size)
        : LinearAllocator(mem.ptr, MIN(size, mem.size))
    {}

    ~LinearAllocator()
    {
        assert(_bytes_allocated == 0);
    }

    LinearAllocator(void* mem, size_t size)
        : _mem(mem)
        , _bytes_allocated(0)
        , _size(size)
    {
        assert(_mem && "should be instantiated with memory");
        assert(_size > 0 && "allocator should have allocated bytes");
    }

    void* Allocate(size_t size) override
    {
        if (!_mem) {
            return nullptr;
        }

        if (size > _size - _bytes_allocated) {
            return nullptr;
        }

        void* free_mem = (void*)((uint8_t*)_mem + _bytes_allocated);
        _bytes_allocated += size;

        return free_mem;
    }

    void Deallocate(void* ptr) override
    { /* Do nothing */
        (void)ptr;
    }

    void Clear() { _bytes_allocated = 0; }

private:
    void* _mem;
    size_t _bytes_allocated;
    size_t _size;
};
