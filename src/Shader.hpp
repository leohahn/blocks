#pragma once

#include "Allocator.hpp"
#include "Collections/String.hpp"
#include "Collections/RobinHashMap.hpp"
#include "Defines.hpp"
#include "Sid.hpp"
#include <assert.h>
#include <glad/glad.h>
#include "Math/Mat4.hpp"
#include "Texture.hpp"

struct Shader
{
    String name;
    uint32_t program;
    RobinHashMap<Sid, int> location_cache;
    mutable bool bound;

public:
    Shader()
        : Shader(nullptr)
    {}
    Shader(Allocator* allocator)
        : name(allocator)
        , program(0)
        , location_cache(allocator, 16)
        , bound(false)
    {}

    ~Shader();

    void Bind() const;
    void Unbind() const;

    void AddUniform(const char* loc);

    bool IsValid() const { return program != 0; }

    void SetUniformMat4(Sid name, const Mat4& mat) const;
    void SetVector(Sid name, const Vec4& vec) const;
    void SetVector(Sid name, const Vec3& vec) const;
    void SetTexture2d(Sid name, const Texture* texture, int texture_index) const;
    void SetTextureIndex(Sid name, int index) const;

    DISABLE_OBJECT_COPY(Shader);

    // move semantics
    Shader(Shader&& other);
    Shader& operator=(Shader&& other);
};

