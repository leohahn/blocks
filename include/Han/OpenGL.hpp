#pragma once

#include "Han/Math/Vec2.hpp"
#include "Han/Math/Vec3.hpp"
#include "Han/Math/Mat4.hpp"
#include <stdlib.h>

namespace OpenGL {

struct Vertex_PT
{
    Vec3 pos;
    Vec2 uv;

    Vertex_PT() = default;

    Vertex_PT(Vec3 pos, Vec2 uv)
        : pos(pos)
        , uv(uv)
    {}
};

static_assert(sizeof(Vertex_PT) == (sizeof(float) * 5), "Should be 5 floats in size");

} // namespace OpenGL
