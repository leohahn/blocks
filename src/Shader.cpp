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
    ASSERT(location != -1, "Uniform does not exist on the shader");

    location_cache.Add(SID(loc), location);
}

void
Shader::SetUniformMat4(const Sid& loc, const Mat4& mat) const
{
    const int* cached_loc = location_cache.Find(loc);
    ASSERT(cached_loc, "value not found");
    glUniformMatrix4fv(*cached_loc, 1, false, &mat.data[0]);
}

void Shader::SetVector(const Sid & loc, const Vec4 & vec) const
{
    const int* cached_loc = location_cache.Find(loc);
    ASSERT(cached_loc, "value not found");
    glUniform4f(*cached_loc, vec.x, vec.y, vec.z, vec.w);
}
