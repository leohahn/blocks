#pragma once

#include "Allocator.hpp"

class Application
{
public:
    Application(Allocator* allocator)
        : _allocator(allocator)
    {}

    virtual ~Application() = default;
    virtual void Update() = 0;

protected:
    Allocator* _allocator;
};