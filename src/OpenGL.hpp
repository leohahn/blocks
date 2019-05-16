#pragma once
#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Mat4.hpp"
#include "glad/glad.h"
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

inline void
SetVertexFormat_PT()
{
    // TODO: consider saving some information here in a Shader, like the position location

    // specify how buffer data is layed out in memory
    int stride = sizeof(Vertex_PT);
    size_t first_byte_offset;
    // pos
    {
        first_byte_offset = 0;
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)first_byte_offset);
        glEnableVertexAttribArray(0);
    }
    // uv
    {
        first_byte_offset = sizeof(decltype(Vertex_PT::pos));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)first_byte_offset);
        glEnableVertexAttribArray(1);
    }
}

inline static void
SetUniformMatrixForCurrentShader(GLuint location, const Mat4& mat)
{
    glUniformMatrix4fv(location, 1, GL_FALSE, &mat.data[0]);
}

} // namespace OpenGL
