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

    OpenGLVertexBuffer(OpenGLVertexBuffer&& other);
    OpenGLVertexBuffer& operator=(OpenGLVertexBuffer&& other);

    OpenGLVertexBuffer(OpenGLVertexBuffer&) = delete;
    OpenGLVertexBuffer& operator=(OpenGLVertexBuffer&) = delete;

private:
    uint32_t _handle;
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
    OpenGLVertexArray();
    ~OpenGLVertexArray();

    void Bind() override;
    void Unbind() override;

    OpenGLVertexArray(OpenGLVertexArray&& other);
    OpenGLVertexArray& operator=(OpenGLVertexArray&& other);

    OpenGLVertexArray(OpenGLVertexArray&) = delete;
    OpenGLVertexArray& operator=(OpenGLVertexArray&) = delete;

private:
    uint32_t _handle;
};
