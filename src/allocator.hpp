#ifndef BLOCKS_ALLOCATOR_HPP
#define BLOCKS_ALLOCATOR_HPP

#include <stddef.h>

class Allocator
{
public:
    virtual void* Allocate(size_t size) = 0;
    virtual void Deallocate(void* ptr) = 0;
};

#endif // BLOCKS_ALLOCATOR_HPP
