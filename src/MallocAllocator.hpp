#pragma once

#include "Allocator.hpp"
#include <assert.h>

class MallocAllocator : public Allocator
{
public:
    MallocAllocator(const char* name)
        : _bytes_allocated(0)
        , _name(name)
    {
        assert(_name && "allocator should have a name");
    }

    ~MallocAllocator() { assert(_bytes_allocated == 0); }

    void* Allocate(size_t size) override
    {
        void* new_mem = malloc(size);
        assert(new_mem);
        _bytes_allocated += size;
        return new_mem;
    }

    void Deallocate(void* ptr) override { free(ptr); }

    const char* GetName() const override { return _name; }

private:
    size_t _bytes_allocated;
    const char* _name;
};
