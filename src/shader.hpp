#pragma once

#include "Allocator.hpp"
#include "Collections/String.hpp"
#include "Defines.hpp"
#include <assert.h>
#include <glad/glad.h>

struct Shader
{
    String name;
    GLuint model_location;
    GLuint view_location;
    GLuint projection_location;
    GLuint program;

public:
    Shader()
        : Shader(nullptr)
    {}
    Shader(Allocator* allocator)
        : name(allocator)
    {}

    ~Shader() { assert(program == 0); }

    void Destroy()
    {
        glDeleteProgram(program);
        program = 0;
    }

    bool IsValid() const { return program != 0; }

    DISABLE_OBJECT_COPY(Shader);
};

inline static void
SetLocationsForShader(Shader* shader)
{
    assert(shader);
    shader->model_location = glGetUniformLocation(shader->program, "model");
    shader->view_location = glGetUniformLocation(shader->program, "view");
    shader->projection_location = glGetUniformLocation(shader->program, "projection");
}
