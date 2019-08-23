#include "Renderer/LowLevelOpenGL.hpp"
#include <glad/glad.h>
#include "Han/Core.hpp"

void
Graphics::LowLevelOpenGLApi::ClearBuffersImpl()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void 
Graphics::LowLevelOpenGLApi::SetFaceCullingImpl(bool on)
{
    if (on) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

void
Graphics::LowLevelOpenGLApi::SetDepthTestImpl(bool on)
{
    if (on) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void
Graphics::LowLevelOpenGLApi::SetViewPortImpl(int x, int y, int width, int height)
{
    glViewport(x, y, (GLsizei)width, (GLsizei)height);
}

void
Graphics::LowLevelOpenGLApi::SetClearColorImpl(const Vec4& color)
{
    glClearColor(color.x, color.y, color.z, color.w);
}

void
Graphics::LowLevelApi::Initialize(Allocator* allocator)
{
    ASSERT(_allocator == nullptr, "Allocator should be null");
    ASSERT(_api == nullptr, "Api should be null");
    _allocator = allocator;
    _api = allocator->New<LowLevelOpenGLApi>();
}

void
Graphics::LowLevelApi::Terminate()
{
    ASSERT(_api, "api should exist");
    ASSERT(_allocator, "allocator should exist");
    _allocator->Delete(_api);
    _allocator = nullptr;
    _api = nullptr;
}

Graphics::LowLevelApi* Graphics::LowLevelApi::_api = nullptr;
Allocator* Graphics::LowLevelApi::_allocator = nullptr;
