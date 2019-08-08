#include "Renderer/LowLevelOpenGL.hpp"
#include <glad/glad.h>
#include "Defines.hpp"

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
Graphics::LowLevelApi::Initialize(Allocator* allocator)
{
    ASSERT(_allocator == nullptr, "Allocator should be null");
    ASSERT(_api == nullptr, "Api should be null");
    _allocator = allocator;
    _api = allocator->New<LowLevelOpenGLApi>();
}

Graphics::LowLevelApi* Graphics::LowLevelApi::_api = nullptr;
Allocator* Graphics::LowLevelApi::_allocator = nullptr;
