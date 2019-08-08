#pragma once

#include "Allocator.hpp"

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

protected:
    virtual ~LowLevelApi() = default;
    virtual void ClearBuffersImpl() = 0;
    virtual void SetFaceCullingImpl(bool on) = 0;
    virtual void SetDepthTestImpl(bool on) = 0;
    virtual void SetViewPortImpl(int x, int y, int width, int height) = 0;

private:
    static Allocator* _allocator;
    static LowLevelApi* _api;
};

}