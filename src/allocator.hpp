#pragma once

#include <stddef.h>

class Allocator
{
public:
    virtual void* Allocate(size_t size) = 0;
    virtual void Deallocate(void* ptr) = 0;
};
