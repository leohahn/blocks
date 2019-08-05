#pragma once

#include <stdint.h>
#include <initializer_list>
#include "Allocator.hpp"
#include "Collections/Array.hpp"

//
// This file is based on the Hazel engine.
//

enum class BufferLayoutDataType
{
    Vec2,
    Vec3,
};

size_t GetLayoutDataTypeSize(BufferLayoutDataType t);
size_t GetLayoutDataTypeNumComponents(BufferLayoutDataType t);

class BufferLayoutElement
{
    friend class BufferLayout;
public:
    BufferLayoutElement(BufferLayoutDataType data_type);
    size_t Offset() const { return _offset; }
    size_t ComponentCount() const { return GetLayoutDataTypeNumComponents(_data_type); }

private:
    BufferLayoutDataType _data_type;
    size_t _offset;
};

class BufferLayout
{
public:
    BufferLayout()
        : _stride(0)
    {}
    BufferLayout(Allocator* allocator, const std::initializer_list<BufferLayoutElement>& data_types);
    size_t Stride() const { return _stride; }
    size_t ElementCount() const { return _elements.len; }

    Array<BufferLayoutElement>::iterator begin() const { return _elements.begin(); }
    Array<BufferLayoutElement>::iterator end() const { return _elements.end(); }

    const BufferLayoutElement& operator[](size_t index) const { return _elements[index]; }

private:
    size_t _stride;
    Array<BufferLayoutElement> _elements;
};

class VertexBuffer
{
public:
    virtual ~VertexBuffer() = default;
    virtual void Bind() = 0;
    virtual void Unbind() = 0;
    virtual void SetLayout(BufferLayout layout) = 0;
    virtual const BufferLayout& Layout() = 0;

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

    virtual void SetVertexBuffer(VertexBuffer* vbo) = 0;
    virtual void SetIndexBuffer(IndexBuffer* ibo) = 0;

    static VertexArray* Create(Allocator* allocator);
};
