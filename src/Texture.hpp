#pragma once

#include "glad/glad.h"
#include "Allocator.hpp"

GLuint LoadTexture(Allocator* allocator, const char* asset_path);
