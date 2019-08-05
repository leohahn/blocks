#pragma once

#include <stdint.h>
#include "Allocator.hpp"

//
// This file is based on the Hazel engine.
//

class VertexBuffer
{
public:
    virtual ~VertexBuffer() = default;
    virtual void Bind() = 0;
    virtual void Unbind() = 0;

    static VertexBuffer* Create(Allocator* allocator, float* data, size_t size);
};

class IndexBuffer
{
public:
    virtual ~IndexBuffer() = default;
    virtual void Bind() = 0;
    virtual void Unbind() = 0;

    static IndexBuffer* Create(Allocator* allocator, uint32_t* indices, size_t len);
};

class VertexArray
{
public:
    virtual ~VertexArray() = default;
    virtual void Bind() = 0;
    virtual void Unbind() = 0;

    static VertexArray* Create(Allocator* allocator);
};
