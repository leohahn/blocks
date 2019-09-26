#pragma once

#include "Han/Allocator.hpp"
#include "Han/Core.hpp"
#include "Han/Logger.hpp"
#include "Han/Memory.hpp"
#include "Han/Utils.hpp"
#include <stdint.h>

class LinearAllocator : public Allocator
{
public:
    LinearAllocator()
        : _bytes_allocated(0)
        , _mem(nullptr)
        , _size(0)
        , _name(nullptr)
    {}

    LinearAllocator(const char* name, const Memory& mem)
        : LinearAllocator(name, mem.ptr, mem.size)
    {}

    LinearAllocator(const char* name, const Memory& mem, size_t size)
        : LinearAllocator(name, mem.ptr, HAN_MIN(size, mem.size))
    {}

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

    LinearAllocator(LinearAllocator&& a)
        : LinearAllocator()
    {
        *this = std::move(a);
    }

    // Move assignment
    LinearAllocator& operator=(LinearAllocator&& a)
    {
        assert(_bytes_allocated == 0);
        _mem = a._mem;
        _size = a._size;
        _name = a._name;
        _bytes_allocated = a._bytes_allocated;
        a._mem = nullptr;
        a._size = 0;
        a._name = nullptr;
        a._bytes_allocated = 0;
        return *this;
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

	size_t GetAllocatedBytes() const override { return _bytes_allocated; }

    const char* GetName() const override { return _name; }

    void Clear() { _bytes_allocated = 0; }

private:
    void* _mem;
    size_t _bytes_allocated;
    size_t _size;
    const char* _name;
};
