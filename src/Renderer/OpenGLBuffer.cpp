#include "OpenGLBuffer.hpp"
#include "Defines.hpp"

#include <glad/glad.h>

OpenGLVertexBuffer::OpenGLVertexBuffer(float* buf, size_t size)
{
    ASSERT(buf, "buffer should exist");
    
    glGenBuffers(1, &_handle);
    glBindBuffer(GL_ARRAY_BUFFER, _handle);
    glBufferData(GL_ARRAY_BUFFER, size, buf, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
    ASSERT(_handle, "handle should exist");
    glDeleteBuffers(1, &_handle);
}

void 
OpenGLVertexBuffer::Bind()
{
    ASSERT(_handle, "handle should exist");
    glBindBuffer(GL_ARRAY_BUFFER, _handle);
}

void 
OpenGLVertexBuffer::Unbind()
{
    ASSERT(_handle, "handle should exist");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// move semantics
OpenGLVertexBuffer::OpenGLVertexBuffer(OpenGLVertexBuffer&& other)
    : _handle(0)
{
    *this = std::move(other);
}

OpenGLVertexBuffer&
OpenGLVertexBuffer::operator=(OpenGLVertexBuffer&& other)
{
    if (_handle) {
        glDeleteBuffers(1, &_handle);
    }
    _handle = other._handle;
    other._handle = 0;
    return *this;
}

//-------------------------------------------------
// Index buffer
//-------------------------------------------------

OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, size_t len)
{
    glGenBuffers(1, &_handle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _handle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 len * sizeof(uint32_t),
                 indices,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer()
{
    ASSERT(_handle, "handle should exist");
    glDeleteBuffers(1, &_handle);
}

void 
OpenGLIndexBuffer::Bind()
{
    ASSERT(_handle, "handle should exist");
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _handle);
}

void 
OpenGLIndexBuffer::Unbind()
{
    ASSERT(_handle, "handle should exist");
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// move semantics
OpenGLIndexBuffer::OpenGLIndexBuffer(OpenGLIndexBuffer&& other)
    : _handle(0)
{
    *this = std::move(other);
}

OpenGLIndexBuffer&
OpenGLIndexBuffer::operator=(OpenGLIndexBuffer&& other)
{
    if (_handle) {
        glDeleteBuffers(1, &_handle);
    }
    _handle = other._handle;
    other._handle = 0;
    return *this;
}

//
// Vertex Array
//

OpenGLVertexArray::OpenGLVertexArray()
{
    glGenVertexArrays(1, &_handle);
}

OpenGLVertexArray::~OpenGLVertexArray()
{
    glDeleteVertexArrays(1, &_handle);
}

void
OpenGLVertexArray::Bind()
{
    ASSERT(_handle, "Handle should exist");
    glBindVertexArray(_handle);
}

void
OpenGLVertexArray::Unbind()
{
    glBindVertexArray(0);
}

OpenGLVertexArray::OpenGLVertexArray(OpenGLVertexArray&& other)
    : _handle(0)
{
    *this = std::move(other);
}

OpenGLVertexArray&
OpenGLVertexArray::operator=(OpenGLVertexArray&& other)
{
    if (_handle) {
        glDeleteVertexArrays(1, &_handle);
    }
    _handle = other._handle;
    other._handle = 0;
    return *this;
}

