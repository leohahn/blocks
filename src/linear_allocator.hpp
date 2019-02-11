#ifndef BLOCKS_LINEAR_ALLOCATOR_HPP
#define BLOCKS_LINEAR_ALLOCATOR_HPP

#include "allocator.hpp"
#include "memory.hpp"

class LinearAllocator : public Allocator
{
public:
    static LinearAllocator Make(Memory mem)
    {
        return Make(mem.ptr, mem.size);
    }

    static LinearAllocator Make(Memory mem, size_t size)
    {
        assert(size <= mem.size);
        return Make(mem.ptr, size);
    }

    static LinearAllocator Make(void* mem, size_t size)
    {
        assert(mem && "should be instantiated with memory");
        assert(size > 0 && "allocator should have allocated bytes");

        LinearAllocator alloc;
        alloc._mem = mem;
        alloc._bytes_allocated = 0;
        alloc._size = size;
        return alloc;
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

    void Deallocate(void* ptr) override {/* Do nothing */ (void)ptr;}

    void Clear() { _bytes_allocated = 0; }

private:
    void* _mem;
    size_t _bytes_allocated;
    size_t _size;
};


#endif // BLOCKS_LINEAR_ALLOCATOR_HPP
