#pragma once

#include "Renderer/LowLevel.hpp"

namespace Graphics
{

class LowLevelOpenGLApi : public LowLevelApi
{
protected:
    void ClearBuffersImpl() override;
    void SetFaceCullingImpl(bool on) override;
    void SetDepthTestImpl(bool on) override;
    void SetViewPortImpl(int x, int y, int width, int height) override;
};

}

