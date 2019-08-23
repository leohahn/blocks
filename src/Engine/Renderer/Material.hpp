#pragma once

#include "Han/Math/Vec3.hpp"
#include "Han/Math/Vec4.hpp"
#include "Han/Math/Mat4.hpp"
#include "Texture.hpp"

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

