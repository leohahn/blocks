#include "Han/Texture.hpp"
#include "glad/glad.h"

void Texture::Destroy()
{
    glDeleteTextures(1, &handle);
}
