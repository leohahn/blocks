#include "Buffer.hpp"
#include "OpenGLBuffer.hpp"
#include "Defines.hpp"

size_t
GetLayoutDataTypeSize(BufferLayoutDataType t)
{
    switch (t) {
        case BufferLayoutDataType::Vec2: return sizeof(float) * 2;
        case BufferLayoutDataType::Vec3: return sizeof(float) * 3;
        default: ASSERT(false, "unknown data type");
    }
    return 0;
}

size_t
GetLayoutDataTypeNumComponents(BufferLayoutDataType t)
{
    switch (t) {
        case BufferLayoutDataType::Vec2: return 2;
        case BufferLayoutDataType::Vec3: return 3;
        default: ASSERT(false, "unknown data type");
    }
    return 0;
}

BufferLayoutElement::BufferLayoutElement(BufferLayoutDataType data_type)
    : _data_type(data_type)
    , _offset(0)
{}

VertexBuffer*
VertexBuffer::Create(Allocator* allocator, float* data, size_t size)
{
    return allocator->New<OpenGLVertexBuffer>(data, size);
}

IndexBuffer*
IndexBuffer::Create(Allocator* allocator, uint32_t* indices, size_t len)
{
    return allocator->New<OpenGLIndexBuffer>(indices, len);
}

VertexArray*
VertexArray::Create(Allocator* allocator)
{
    return allocator->New<OpenGLVertexArray>(allocator);
}


BufferLayout::BufferLayout(Allocator* allocator, const std::initializer_list<BufferLayoutElement>& elements)
    : _elements(allocator, elements)
    , _stride(0)
{
    size_t offset = 0;

    for (auto& el : _elements) {
        el._offset = offset;
        size_t size = GetLayoutDataTypeSize(el._data_type);
        offset += size;
        _stride += size;
    }
}
