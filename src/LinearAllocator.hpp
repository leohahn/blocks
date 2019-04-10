#pragma once

#include "Allocator.hpp"
#include "Defines.hpp"
#include "Logger.hpp"
#include "Memory.hpp"
#include "Utils.hpp"

class LinearAllocator : public Allocator
{
public:
    LinearAllocator(const char* name, Memory mem)
        : LinearAllocator(name, mem.ptr, mem.size)
    {}

    LinearAllocator(const char* name, Memory mem, size_t size)
        : LinearAllocator(name, mem.ptr, MIN(size, mem.size))
    {}

    ~LinearAllocator() { assert(_bytes_allocated == 0); }

    LinearAllocator(const char* name, void* mem, size_t size)
        : _mem(mem)
        , _bytes_allocated(0)
        , _size(size)
        , _name(name)
    {
        assert(_mem && "should be instantiated with memory");
        assert(_size > 0 && "allocator should have allocated bytes");
        assert(_name && "allocator should have a name");
    }

    void* Allocate(size_t size) override
    {
        assert(_mem);

        if (size > _size - _bytes_allocated) {
            LOG_WARN("Cannot allocate %s memory in %s allocator (size of %s)",
                     Utils::GetPrettySize(size),
                     _name,
                     Utils::GetPrettySize(_size));
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

    const char* GetName() const override { return _name; }

    void Clear() { _bytes_allocated = 0; }

private:
    void* _mem;
    size_t _bytes_allocated;
    size_t _size;
    const char* _name;
};
