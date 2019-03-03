#ifndef BLOCKS_SHADER_HPP
#define BLOCKS_SHADER_HPP

#include <glad/glad.h>
#include "linear_allocator.hpp"

namespace Shader
{

GLuint LoadFromFile(const char* path, LinearAllocator allocator);

}

#endif // BLOCKS_SHADER_HPP
