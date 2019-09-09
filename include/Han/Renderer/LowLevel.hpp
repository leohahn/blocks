#pragma once

#include "Han/Allocator.hpp"
#include "Han/Math/vec4.hpp"

namespace Graphics
{

class LowLevelApi
{
public:
    static void Initialize(Allocator* allocator);
    static void ClearBuffers() { _api->ClearBuffersImpl(); }
    static void SetFaceCulling(bool on) { _api->SetFaceCullingImpl(on); }
    static void SetDepthTest(bool on) { _api->SetDepthTestImpl(on); }
    static void SetViewPort(int x, int y, int width, int height) { _api->SetViewPortImpl(x, y, width, height); }
    static void Terminate();
    static void SetClearColor(const Vec4& color) { _api->SetClearColorImpl(color); }
    virtual ~LowLevelApi() = default;

protected:
    virtual void ClearBuffersImpl() = 0;
    virtual void SetFaceCullingImpl(bool on) = 0;
    virtual void SetDepthTestImpl(bool on) = 0;
    virtual void SetViewPortImpl(int x, int y, int width, int height) = 0;
    virtual void SetClearColorImpl(const Vec4& color) = 0;

private:
    static Allocator* _allocator;
    static LowLevelApi* _api;
};

}