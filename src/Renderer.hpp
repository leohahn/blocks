#pragma once

#include "Allocator.hpp"
#include "Array.hpp"

struct Mesh
{
    Array<int> meshes;
};

class Renderer
{
public:
    Renderer()
    : _allocator(nullptr)
    {}

private:
    Allocator* _allocator;
};