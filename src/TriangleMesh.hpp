#pragma once

#include "Collections/Array.hpp"
#include "Collections/String.hpp"
#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec4.hpp"
#include "Texture.hpp"
#include "Sid.hpp"

// This enumeration specifies how a material should be rendered.
enum class IlluminationModel
{
    Color = 0,
    Diffuse = 1,
    DiffuseAndSpecular = 2,
};

struct Material
{
    Sid name;
    IlluminationModel illumination_model;

    Vec3 diffuse_color = Vec3::Zero();
    Vec3 ambient_color = Vec3::Zero();
    Vec3 specular_color = Vec3::Zero();
    float shininess = 0.0f;
    Texture* diffuse_map = nullptr;
    Texture* normal_map = nullptr;
    Texture* specular_map = nullptr;
};

struct SubMesh
{
    // Rendering information
    int32_t start_index;
    size_t num_indices;
    Material* material;

    // Placement information
    Vec3 local_position;
    Quaternion local_orientation;
};

struct TriangleMesh
{
    // TODO: probably need some flags here
    // uint32_t checksum;
    Sid name;

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
        : vertices(allocator)
        , uvs(allocator)
        , colors(allocator)
        , normals(allocator)
        , indices(allocator)
        , sub_meshes(allocator)
        , vao(0)
        , vbo(0)
        , ebo(0)
    {}
};

