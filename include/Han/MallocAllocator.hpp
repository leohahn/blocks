#pragma once

#include "Allocator.hpp"
#include <assert.h>

class MallocAllocator : public Allocator
{
public:
    MallocAllocator()
        : MallocAllocator("")
    {}

    MallocAllocator(const char* name)
        : _bytes_water_mark(0)
        , _name(name)
    {
        assert(_name && "allocator should have a name");
    }

    void* Allocate(size_t size) override
    {
        void* new_mem = malloc(size);
        assert(new_mem);
        _bytes_water_mark += size;
        return new_mem;
    }

    void Deallocate(void* ptr) override { free(ptr); }

    const char* GetName() const override { return _name; }

    size_t GetBytesWaterMark() const { return _bytes_water_mark; }

	static Allocator* Instance()
	{
		static MallocAllocator alloc;
		return &alloc;
	}

private:
    size_t _bytes_water_mark;
    const char* _name;
};
