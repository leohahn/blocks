#pragma once

#include "Buffer.hpp"
#include <stdint.h>

class OpenGLVertexBuffer : public VertexBuffer
{
public:
    OpenGLVertexBuffer(float* buf, size_t size);
    ~OpenGLVertexBuffer();

    void Bind() override;
    void Unbind() override;
    void SetLayout(BufferLayout layout) override;
    const BufferLayout& Layout() override;

    OpenGLVertexBuffer(OpenGLVertexBuffer&& other);
    OpenGLVertexBuffer& operator=(OpenGLVertexBuffer&& other);

    OpenGLVertexBuffer(OpenGLVertexBuffer&) = delete;
    OpenGLVertexBuffer& operator=(OpenGLVertexBuffer&) = delete;

private:
    uint32_t _handle;
    BufferLayout _layout;
};

class OpenGLIndexBuffer : public IndexBuffer
{
public:
    OpenGLIndexBuffer(uint32_t* indices, size_t len);
    ~OpenGLIndexBuffer();

    void Bind() override;
    void Unbind() override;

    OpenGLIndexBuffer(OpenGLIndexBuffer&& other);
    OpenGLIndexBuffer& operator=(OpenGLIndexBuffer&& other);

    OpenGLIndexBuffer(OpenGLIndexBuffer&) = delete;
    OpenGLIndexBuffer& operator=(OpenGLIndexBuffer&) = delete;

private:
    uint32_t _handle;
};

class OpenGLVertexArray : public VertexArray
{
public:
    OpenGLVertexArray(Allocator* allocator);
    ~OpenGLVertexArray();

    void Bind() override;
    void Unbind() override;

    void SetVertexBuffer(VertexBuffer* vbo) override;
    void SetIndexBuffer(IndexBuffer* ibo) override;

    OpenGLVertexArray(OpenGLVertexArray&& other);
    OpenGLVertexArray& operator=(OpenGLVertexArray&& other);

    OpenGLVertexArray(OpenGLVertexArray&) = delete;
    OpenGLVertexArray& operator=(OpenGLVertexArray&) = delete;

private:
    Allocator* _allocator;
    uint32_t _handle;
    VertexBuffer* _vbo;
    IndexBuffer* _ibo;
};
