#pragma once

#include "Allocator.hpp"
#include "Collections/String.hpp"
#include "Collections/RobinHashMap.hpp"
#include "Defines.hpp"
#include "Sid.hpp"
#include <assert.h>
#include <glad/glad.h>
#include "Math/Mat4.hpp"

struct Shader
{
    String name;
    uint32_t program;
    RobinHashMap<Sid, int> location_cache;

public:
    Shader()
        : Shader(nullptr)
    {}
    Shader(Allocator* allocator)
        : name(allocator)
        , program(0)
        , location_cache(allocator, 16)
    {}

    ~Shader();

    void Bind() const;
    void Unbind() const;

    void AddUniform(const char* loc);

    bool IsValid() const { return program != 0; }

    void SetUniformMat4(const Sid& loc, const Mat4& mat) const;
    void SetVector(const Sid& loc, const Vec4& vec) const;

    DISABLE_OBJECT_COPY(Shader);

    // move semantics
    Shader(Shader&& other);
    Shader& operator=(Shader&& other);
};

