#pragma once

#include "Collections/Array.hpp"
#include "Collections/String.hpp"
#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec4.hpp"
#include "Texture.hpp"

// TODO: should these be compressed? (e.g., quaternion)
struct Frame3
{
    Vec3 tangent;
    Vec3 bitangent;
    Vec3 normal;
};

struct TriangleMesh
{
    // TODO: probably need some flags here
    // uint32_t checksum;
    String name;
    String full_path;
    String source_file_path;

    Array<Vec3> vertices;
    Array<Vec2> uvs;
    Array<Vec4> colors;
    Array<Frame3> vertex_frames;

    Array<int32_t> indices;
    
    // OpenGL state
    uint32_t vao;
    uint32_t vbo;
    uint32_t ebo;
    
    TriangleMesh(Allocator* allocator)
        : name(allocator)
        , full_path(allocator)
        , source_file_path(allocator)
        , vertices(allocator)
        , uvs(allocator)
        , colors(allocator)
        , vertex_frames(allocator)
        , indices(allocator)
        , vao(0)
        , vbo(0)
        , ebo(0)
    {}

    void Destroy()
    {
        name.Destroy();
        full_path.Destroy();
        source_file_path.Destroy();
        vertices.Destroy();
        uvs.Destroy();
        colors.Destroy();
        vertex_frames.Destroy();
        indices.Destroy();
    }
};

