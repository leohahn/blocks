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

void 
OpenGLVertexBuffer::SetLayout(BufferLayout layout)
{
    ASSERT(layout.ElementCount() > 0, "Layout should not be empty");
    _layout = std::move(layout);
}

const BufferLayout&
OpenGLVertexBuffer::Layout()
{
    return _layout;
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
    _layout = std::move(other._layout);
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

OpenGLVertexArray::OpenGLVertexArray(Allocator* allocator)
    : _allocator(allocator)
    , _vbo(nullptr)
    , _ibo(nullptr)
{
    glGenVertexArrays(1, &_handle);
}

OpenGLVertexArray::~OpenGLVertexArray()
{
    if (_allocator) {
        _allocator->Delete(_vbo);
        _allocator->Delete(_ibo);
    }
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
    : _allocator(nullptr)
    , _handle(0)
    , _vbo(nullptr)
    , _ibo(nullptr)
{
    *this = std::move(other);
}

OpenGLVertexArray&
OpenGLVertexArray::operator=(OpenGLVertexArray&& other)
{
    if (_allocator) {
        _allocator->Delete(_vbo);
        _allocator->Delete(_ibo);
    }
    if (_handle) {
        glDeleteVertexArrays(1, &_handle);
    }
    _allocator = other._allocator;
    _handle = other._handle;
    _vbo = other._vbo;
    _ibo = other._ibo;
    other._allocator = nullptr;
    other._handle = 0;
    other._vbo = nullptr;
    other._ibo = nullptr;
    return *this;
}

void
OpenGLVertexArray::SetVertexBuffer(VertexBuffer* vbo)
{
    ASSERT(vbo, "there should be a Vertex Buffer");
    ASSERT(_allocator, "there should be an allocator set");
    ASSERT(_handle, "there should be a handle set");

    _allocator->Delete(_vbo);
    _vbo = vbo;

    const BufferLayout& layout = _vbo->Layout();

    ASSERT(layout.ElementCount() > 0, "Layout is empty!");

    {
        glBindVertexArray(_handle);
        _vbo->Bind();

        for (size_t li = 0; li < layout.ElementCount(); ++li) {
            const BufferLayoutElement& el = layout[li];
            glVertexAttribPointer(
                li,
                el.ComponentCount(),
                GL_FLOAT, // TODO: remove hardcoded enum
                GL_FALSE,
                layout.Stride(),
                (void*)el.Offset());
            glEnableVertexAttribArray(li);
        }
        glBindVertexArray(0);
        _vbo->Unbind();
    }
}

void
OpenGLVertexArray::SetIndexBuffer(IndexBuffer* ibo)
{
    ASSERT(ibo, "there should be an Index Buffer");
    ASSERT(_allocator, "there should be an allocator set");
    ASSERT(_handle, "there should be a handle set");

    _allocator->Delete(_ibo);
    _ibo = ibo;

    glBindVertexArray(_handle);
    _ibo->Bind();
    glBindVertexArray(0);
    _ibo->Unbind();
}
