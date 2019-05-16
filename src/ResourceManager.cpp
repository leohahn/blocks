#include "ResourceManager.hpp"

#include "FileSystem.hpp"
#include "Logger.hpp"
#include "OpenGL.hpp"
#include "Path.hpp"
#include "glad/glad.h"
#include "stb_image.h"

static Texture* LoadTextureFromFile(Allocator* allocator,
                                    Allocator* scratch_allocator,
                                    const StringView& texture_file);

void
ResourceManager::Create()
{
    resources_path = FileSystem::GetResourcesPath(allocator);
}

void
ResourceManager::Destroy()
{
    for (size_t i = 0; i < meshes.len; ++i) {
        meshes[i]->Destroy();
        allocator->Delete(meshes[i]);
    }
    meshes.Destroy();

    for (size_t i = 0; i < textures.len; ++i) {
        textures[i]->Destroy();
        allocator->Delete(textures[i]);
    }
    textures.Destroy();

    for (size_t i = 0; i < shaders.len; ++i) {
        shaders[i]->Destroy();
        allocator->Delete(shaders[i]);
    }
    shaders.Destroy();

    resources_path.Destroy();

    allocator = nullptr;
    scratch_allocator = nullptr;
}

Model
ResourceManager::LoadModel(const StringView& model_file)
{
    LOG_INFO("Loading mesh %s", model_file.data);

    Path full_path(scratch_allocator);
    full_path.Push(resources_path);
    full_path.Push(model_file);

	return Model(allocator);
}

void
ResourceManager::LoadTexture(const StringView& texture_file)
{
    assert(texture_file.data[texture_file.len] == 0);
    Texture* new_texture = LoadTextureFromFile(allocator, scratch_allocator, texture_file);
    textures.PushBack(new_texture);
}

void
ResourceManager::LoadShader(const StringView& shader_file)
{
    Path full_path(scratch_allocator);
    full_path.Push(resources_path);
    full_path.Push("shaders");
    full_path.Push(shader_file);

    LOG_DEBUG("Making shader program for %s\n", shader_file.data);

    Shader* shader = allocator->New<Shader>(allocator);
    assert(shader);
    shader->name.Append(shader_file);

    GLuint vertex_shader = 0, fragment_shader = 0;
    GLchar info[512] = {};
    GLint success = false;

    char* shader_string = nullptr;
    {
        FILE* fp = fopen(full_path.data, "rb");
        assert(fp && "shader file should exist");

        // take the size of the file
        fseek(fp, 0, SEEK_END);
        size_t file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        // allocate enough memory
        shader_string = (char*)allocator->Allocate(file_size);
        assert(shader_string && "there should be enough memory here");

        // read the file into the buffer
        size_t nread = fread(shader_string, 1, file_size, fp);
        assert(nread == file_size && "the correct amount of bytes should be read");

        fclose(fp);
    }

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    if (vertex_shader == 0 || fragment_shader == 0) {
        LOG_ERROR("failed to create shaders (glCreateShader)\n");
        goto error_cleanup;
    }

    {
        const char *vertex_string[3] = {
            "#version 330 core\n",
            "#define VERTEX_SHADER\n",
            shader_string,
        };
        glShaderSource(vertex_shader, 3, &vertex_string[0], nullptr);

        const char *fragment_string[3] = {
            "#version 330 core\n",
            "#define FRAGMENT_SHADER\n",
            shader_string,
        };
        glShaderSource(fragment_shader, 3, &fragment_string[0], nullptr);
    }

    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info);
        LOG_ERROR("Vertex shader compilation failed: %s\n", info);
        goto error_cleanup;
    }

    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info);
        LOG_ERROR("Fragment shader compilation failed: %s\n", info);
        goto error_cleanup;
    }

    shader->program = glCreateProgram();
    glAttachShader(shader->program, vertex_shader);
    glAttachShader(shader->program, fragment_shader);
    glLinkProgram(shader->program);
    glGetProgramiv(shader->program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader->program, 512, nullptr, info);
        LOG_ERROR("Shader linking failed: %s\n", info);
        goto error_cleanup;
    }
    
    goto ok;
    
error_cleanup:
    LOG_ERROR("Failed to load shader %s", shader_file.data);
    allocator->Delete(shader);

ok:
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    full_path.Destroy();
    shaders.PushBack(shader);
}


//================================================================
// Helper functions
//================================================================

static Texture*
LoadTextureFromFile(Allocator* allocator,
                    Allocator* scratch_allocator,
                    const StringView& texture_file)
{
    assert(allocator);
    assert(scratch_allocator);

    Path resources_path = FileSystem::GetResourcesPath(scratch_allocator);

    Path full_asset_path(scratch_allocator);
    full_asset_path.Push(resources_path);
    full_asset_path.Push(texture_file);

    Texture* texture = allocator->New<Texture>(allocator);
    texture->name.Append(texture_file);

    size_t texture_buffer_size;
    uint8_t* texture_buffer =
        FileSystem::LoadFileToMemory(allocator, full_asset_path, &texture_buffer_size);
    int texture_width, texture_height, texture_channel_count;

    assert(texture_buffer);

    // memory was read, now load it into an opengl buffer

    // tell stb_image.h to flip loaded texture's on the y-axis.
    stbi_set_flip_vertically_on_load(true);
    uint8_t* data = stbi_load_from_memory(texture_buffer,
                                          (int)texture_buffer_size,
                                          &texture_width,
                                          &texture_height,
                                          &texture_channel_count,
                                          0);

    texture->width = texture_width;
    texture->height = texture_height;

    if (!data) {
        LOG_ERROR("Failed to load texture at %s", full_asset_path.data);
        goto cleanup_texture_buffer;
    }

    glGenTextures(1, &texture->handle);
    glBindTexture(GL_TEXTURE_2D, texture->handle);

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
    full_asset_path.Destroy();
    resources_path.Destroy();

    texture->loaded = true;
    return texture;
}
