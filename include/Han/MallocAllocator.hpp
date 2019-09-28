#pragma once

#include "Han/Core.hpp"
#include "Han/Allocator.hpp"
#include <limits>
#include <stdint.h>

class MallocAllocator : public Allocator
{
public:
    MallocAllocator()
        : MallocAllocator("Malloc")
    {}

    MallocAllocator(const char* name)
        : _bytes_water_mark(0)
		, _bytes_allocated(0)
        , _name(name)
    {
    }

    void* Allocate(size_t size) override
    {
		ASSERT(size >= 0 && size <= std::numeric_limits<int32_t>::max(), "Allocation should be within bounds");
		size_t size_with_header = size + sizeof(int32_t);
        void* new_mem = malloc(size_with_header);
        ASSERT(new_mem, "Not enough memory");

		// write header
		*((int32_t*)new_mem) = size;
        _bytes_water_mark += size;
		_bytes_allocated += size;
        return (int32_t*)new_mem + 1;
    }

    void Deallocate(void* ptr) override
	{
		if (ptr) {
			void* header = (int32_t*)ptr - 1;
			int32_t size = *((int32_t*)header);
			_bytes_allocated -= size;
			free(header);
		}
	}

    const char* GetName() const override { return _name; }

    size_t GetBytesWaterMark() const { return _bytes_water_mark; }
	size_t GetAllocatedBytes() const override { return _bytes_allocated; }

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
	size_t _bytes_allocated;
    const char* _name;
};
