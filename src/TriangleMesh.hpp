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

struct TriangleListInfo
{
    // int32_t material_index; // which material used for rendering this?
    int32_t num_indices = 0;
    int32_t first_index = 0;
    Texture* texture = nullptr;
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
    Array<TriangleListInfo> triangle_list_infos;
    
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
        , triangle_list_infos(allocator)
        , vao(0)
        , vbo(0)
        , ebo(0)
    {}
};


