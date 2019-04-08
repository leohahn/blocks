#pragma once

#include "Allocator.hpp"
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
    int32_t num_indices;
    int32_t first_index;
    Texture* texture;
};

struct TriangleMesh
{
    // uint32_t checksum;
    Allocator* allocator;
    String name;
    String full_path;
    String source_file_path;

    Array<Vec3> vertices;
    Array<Vec2> uvs;
    Array<Vec4> colors;
    Array<Frame3> vertex_frames;

    Array<int32_t> indexes;
    Array<TriangleListInfo> triangle_list_infos;
};
