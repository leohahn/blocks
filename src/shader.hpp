#pragma once

#include <glad/glad.h>
#include "LinearAllocator.hpp"

namespace Shader
{

GLuint LoadFromFile(const char* path, Allocator* allocator);

}
