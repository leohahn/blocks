#include "Buffer.hpp"
#include "OpenGLBuffer.hpp"

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
    return allocator->New<OpenGLVertexArray>();
}
