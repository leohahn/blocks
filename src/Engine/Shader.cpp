#include "Shader.hpp"
#include "glad/glad.h"

Shader::~Shader()
{
    glDeleteProgram(program);
}

void
Shader::Bind() const
{
    glUseProgram(program);
    bound = true;
}

void
Shader::Unbind() const
{ 
    glUseProgram(0);
    bound = false;
} 

Shader::Shader(Shader&& other)
    : Shader()
{
    *this = std::move(other);
}

Shader&
Shader::operator=(Shader&& other)
{
    if (program) {
        glDeleteProgram(program);
    }
    program = other.program;
    name = std::move(other.name);
    location_cache = std::move(other.location_cache);
    other.program = 0;
    return *this;
}

void
Shader::AddUniform(const char* loc)
{
    ASSERT(loc, "Location is null!");
    ASSERT(location_cache.Find(SID(loc)) == nullptr, "Location already added!");

    int location = glGetUniformLocation(program, loc);
    if (location == -1) {
        LOG_ERROR("Uniform %s does not exist on the shader", loc);
        return;
    }

    location_cache.Add(SID(loc), location);
}

void
Shader::SetUniformMat4(Sid loc, const Mat4& mat) const
{
    ASSERT(bound, "shader should be bound");
    const int* cached_loc = location_cache.Find(loc);
    if (cached_loc) {
        glUniformMatrix4fv(*cached_loc, 1, false, &mat.data[0]);
    }
}

void
Shader::SetVector(Sid loc, const Vec4 & vec) const
{
    ASSERT(bound, "shader should be bound");
    const int* cached_loc = location_cache.Find(loc);
    if (cached_loc) {
        glUniform4f(*cached_loc, vec.x, vec.y, vec.z, vec.w);
    }
}

void
Shader::SetVector(Sid loc, const Vec3& vec) const
{
    ASSERT(bound, "shader should be bound");
    const int* cached_loc = location_cache.Find(loc);
    if (cached_loc) {
        glUniform3f(*cached_loc, vec.x, vec.y, vec.z);
    }
}

void
Shader::SetTexture2d(Sid loc, const Texture* texture, int texture_index) const
{
    ASSERT(bound, "shader should be bound");
    const int* cached_loc = location_cache.Find(loc);
    if (cached_loc) {
        glActiveTexture(GL_TEXTURE0 + texture_index);
        glBindTexture(GL_TEXTURE_2D, texture->handle);
    }
}

void
Shader::SetTextureIndex(Sid name, int index) const
{
    ASSERT(bound, "shader should be bound");
    const int* cached_loc = location_cache.Find(name);
    if (cached_loc) {
        glUniform1i(*cached_loc, index);
    } else {
        UNREACHABLE;
    }
}

