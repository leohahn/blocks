#include "Texture.hpp"

#include "FileSystem.hpp"
#include "Logger.hpp"
#include "stb_image.h"
#include <assert.h>

GLuint
LoadTexture(Allocator* allocator, const char* asset_path)
{
    GLuint texture;
    size_t texture_buffer_size;
    uint8_t* texture_buffer =
        FileSystem::LoadFileToMemory(allocator, asset_path, &texture_buffer_size);
    int texture_width, texture_height, texture_channel_count;

    assert(texture_buffer);

    // memory was read, now load it into an opengl buffer

    // load image, create texture and generate mipmaps

    stbi_set_flip_vertically_on_load(
        true); // tell stb_image.h to flip loaded texture's on the y-axis.
    uint8_t* data = stbi_load_from_memory(texture_buffer,
                                          texture_buffer_size,
                                          &texture_width,
                                          &texture_height,
                                          &texture_channel_count,
                                          0);

    if (!data) {
        LOG_ERROR("Failed to load texture at %s", asset_path);
        goto cleanup_texture_buffer;
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S,
                    GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    free(data);

cleanup_texture_buffer:
    allocator->Deallocate(texture_buffer);

    return texture;
}
