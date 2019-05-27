#pragma once

#include "Collections/Array.hpp"
#include "Collections/String.hpp"
#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec4.hpp"
#include "Texture.hpp"

struct Material
{
    // TODO: figure out important stuff to put here.
    float diffure_color;
};

struct SubMesh
{
    // Rendering information
    int32_t start_index;
    size_t num_indices;
    Material material;

    // Placement information
    Vec3 local_position;
    Quaternion local_orientation;
};

struct TriangleMesh
{
    // TODO: probably need some flags here
    // uint32_t checksum;
    String name;

    Array<Vec3> vertices;
    Array<Vec2> uvs;
    Array<Vec4> colors;
    Array<Vec3> normals;
    Array<int32_t> indices;

    Array<SubMesh> sub_meshes;
    
    // OpenGL state
    uint32_t vao;
    uint32_t vbo;
    uint32_t ebo;
    
    TriangleMesh(Allocator* allocator)
        : name(allocator)
        , vertices(allocator)
        , uvs(allocator)
        , colors(allocator)
        , normals(allocator)
        , indices(allocator)
        , sub_meshes(allocator)
        , vao(0)
        , vbo(0)
        , ebo(0)
    {}

    void Destroy()
    {
        vertices.Destroy();
        uvs.Destroy();
        colors.Destroy();
        normals.Destroy();
        indices.Destroy();
        sub_meshes.Destroy();
    }
};

