#pragma once

#include <assert.h>
#include <glad/glad.h>
#include "LinearAllocator.hpp"

struct Shader
{
    // TODO: implement a Destroy function
    GLuint model_location;
    GLuint view_location;
    GLuint projection_location;
    GLuint program;

    bool IsValid() const { return program != 0; }
    static Shader LoadFromFile(const char* path, Allocator* allocator);
};

inline static void
SetLocationsForShader(Shader* shader)
{
    assert(shader);
    shader->model_location = glGetUniformLocation(shader->program, "model");
    shader->view_location = glGetUniformLocation(shader->program, "view");
    shader->projection_location = glGetUniformLocation(shader->program, "projection");
}
