#pragma once

#include "Collections/Array.hpp"
#include "Collections/String.hpp"
#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec4.hpp"
#include "Texture.hpp"
#include "Sid.hpp"
#include "Shader.hpp"
#include "Renderer/Buffer.hpp"

// This enumeration specifies how a material should be rendered.
enum class IlluminationModel
{
    Color = 0,
    Diffuse = 1,
    DiffuseAndSpecular = 2,
};

class MaterialValue
{
    friend struct Material;
public:
    enum class Kind
    {
        Vec3, Vec4, Mat4, Texture
    };

    MaterialValue(Vec3 vec3)
        : _kind(Kind::Vec3)
        , _vec3(vec3)
    {}

    MaterialValue(Vec4 vec4)
        : _kind(Kind::Vec4)
        , _vec4(vec4)
    {}

    MaterialValue(Mat4 mat4)
        : _kind(Kind::Mat4)
        , _mat4(mat4)
    {}

    MaterialValue(Texture* tex)
        : _kind(Kind::Texture)
    {
        ASSERT(tex, "tex is null!");
        _texture.ptr = tex;
        _texture.shader_index = -1;
    }

    Kind GetKind() const { return _kind; }
    Vec3 GetVec3() const { return _vec3; }
    Vec4 GetVec4() const { return _vec4; }
    Mat4 GetMat4() const { return _mat4; }
    Texture* GetTexture() const { return _texture.ptr; }
    int GetTextureIndex() const { return _texture.shader_index; }
private:
    Kind _kind;
    union
    {
        Vec3 _vec3;
        Vec4 _vec4;
        Mat4 _mat4;
        struct
        {
            Texture* ptr;
            int shader_index;
        } _texture;
    };
private:
};

struct Material
{
public:
    Sid name;
    IlluminationModel illumination_model;

    Vec3 diffuse_color = Vec3::Zero();
    Vec3 ambient_color = Vec3::Zero();
    Vec3 specular_color = Vec3::Zero();
    float shininess = 0.0f;
    Shader* shader = nullptr;
    RobinHashMap<Sid, MaterialValue> values;
private:
    int _next_index = 0;

public:

    Material(Allocator* allocator)
        : values(allocator, 16)
        , _next_index(0)
    {}

    void AddValue(Sid name, MaterialValue val)
    {
        if (val._kind == MaterialValue::Kind::Texture) {
            val._texture.shader_index = _next_index++;
            shader->SetTextureIndex(name, val._texture.shader_index);
        }
        values.Add(name, val);
    }

    void Bind()
    {
        ASSERT(shader, "shader is null!");
        for (const auto& pair : values) {
            switch (pair.val.GetKind()) {
                case MaterialValue::Kind::Vec3:
                    shader->SetVector(pair.key, pair.val.GetVec3());
                    break;
                case MaterialValue::Kind::Vec4:
                    shader->SetVector(pair.key, pair.val.GetVec4());
                    break;
                case MaterialValue::Kind::Mat4:
                    shader->SetUniformMat4(pair.key, pair.val.GetMat4());
                    break;
                case MaterialValue::Kind::Texture:
                    shader->SetTexture2d(pair.key, pair.val.GetTexture(), pair.val.GetTextureIndex());
                    break;
                default:
                    ASSERT(false, "unrecognized kind");
                    break;
            }
        }
    }
};

struct SubMesh
{
    // Rendering information
    int32_t start_index;
    size_t num_indices;

    // Placement information
    Vec3 local_position;
    Quaternion local_orientation;
    
    VertexArray* vao;
    Material* material;
};

struct TriangleMesh
{
    // TODO: probably need some flags here
    // uint32_t checksum;
    Allocator* allocator;
    Sid name;

    Array<Vec3> vertices;
    Array<Vec2> uvs;
    Array<Vec4> colors;
    Array<Vec3> normals;
    Array<uint32_t> indices;

    Array<SubMesh> sub_meshes;
    
    TriangleMesh(Allocator* allocator)
        : allocator(allocator)
        , vertices(allocator)
        , uvs(allocator)
        , colors(allocator)
        , normals(allocator)
        , indices(allocator)
        , sub_meshes(allocator)
    {}

    ~TriangleMesh()
    {
        for (auto& sm : sub_meshes) {
            allocator->Delete(sm.vao);
        }
    }

    TriangleMesh(TriangleMesh&& other)
        : allocator(nullptr)
    {
        *this = std::move(other);
    }

    TriangleMesh& operator=(TriangleMesh&& other)
    {
        if (allocator) {
            for (auto& sm : sub_meshes) {
                allocator->Delete(sm.vao);
            }
        }
        allocator = other.allocator;
        vertices = std::move(other.vertices);
        uvs = std::move(other.uvs);
        colors = std::move(other.colors);
        normals = std::move(other.normals);
        indices = std::move(other.indices);
        sub_meshes = std::move(other.sub_meshes);
        return *this;
    }

    TriangleMesh(const TriangleMesh& other) = delete;
    TriangleMesh& operator=(const TriangleMesh& other) = delete;
};

