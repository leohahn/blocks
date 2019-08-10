#include "Shader.hpp"

#include "Shader.hpp"

Shader::~Shader()
{
    glDeleteProgram(program);
}

void
Shader::Bind() const
{
    glUseProgram(program);
}

void
Shader::Unbind() const
{ 
    glUseProgram(0);
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
    const int* cached_loc = location_cache.Find(loc);
    ASSERT(cached_loc, "value not found");
    glUniformMatrix4fv(*cached_loc, 1, false, &mat.data[0]);
}

void
Shader::SetVector(Sid loc, const Vec4 & vec) const
{
    const int* cached_loc = location_cache.Find(loc);
    ASSERT(cached_loc, "value not found");
    glUniform4f(*cached_loc, vec.x, vec.y, vec.z, vec.w);
}

void
Shader::SetVector(Sid loc, const Vec3& vec) const
{
    const int* cached_loc = location_cache.Find(loc);
    ASSERT(cached_loc, "value not found");
    glUniform3f(*cached_loc, vec.x, vec.y, vec.z);
}

void
Shader::SetTexture2d(Sid loc, const Texture* texture, int texture_index) const
{
    const int* cached_loc = location_cache.Find(loc);
    if (!cached_loc) {
        return;
    }
    glActiveTexture(GL_TEXTURE0 + texture_index);
    glBindTexture(GL_TEXTURE_2D, texture->handle);
}

void
Shader::SetTextureIndex(Sid name, int index) const
{
    const int* cached_loc = location_cache.Find(name);
    if (!cached_loc) {
        return;
    }
    glUniform1i(*cached_loc, index);
}

