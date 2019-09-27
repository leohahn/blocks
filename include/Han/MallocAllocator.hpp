#pragma once

#include "Allocator.hpp"
#include <assert.h>

class MallocAllocator : public Allocator
{
public:
    MallocAllocator()
        : MallocAllocator("Malloc")
    {}

    MallocAllocator(const char* name)
        : _bytes_water_mark(0)
        , _name(name)
    {
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
	size_t GetAllocatedBytes() const override { return _bytes_water_mark; }

	// Malloc allocators have no size, since the amount of memory is only bounded
	// by the operating system.
	size_t GetSize() const override { return 0; }
	AllocatorType GetType() const override { return AllocatorType::Malloc; }

	static Allocator* Instance()
	{
		static MallocAllocator alloc;
		return &alloc;
	}

private:
    size_t _bytes_water_mark;
    const char* _name;
};
