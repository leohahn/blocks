#pragma once

#include "Allocator.hpp"
#include "Collections/Array.hpp"
#include "Defines.hpp"
#include "Math/Quaternion.hpp"
#include "Path.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "TriangleMesh.hpp"

struct Model
{
    Array<TriangleMesh*> meshes;
    Vec3 position;
    Quaternion orientation;
    float scale;

public:
    Model(Allocator* allocator)
        : meshes(allocator)
    {}
    
    Model(Model&& model) = default;

    void Destroy()
    {
        // The model does not own the triangle meshes, therefore it does not
        // destroy them.
        meshes.Destroy();
    }
    
    DISABLE_OBJECT_COPY(Model);
};

struct ResourceManager
{
    Allocator* allocator;
    Allocator* scratch_allocator;
    Array<TriangleMesh*> meshes;
    Array<Texture*> textures;
    Array<Shader*> shaders;
    Path resources_path;

public:
    ResourceManager(Allocator* allocator, Allocator* scratch_allocator)
        : allocator(allocator)
        , scratch_allocator(scratch_allocator)
        , meshes(allocator)
        , textures(allocator)
        , shaders(allocator)
        , resources_path(allocator)
    {}

    void Create();
    void Destroy();

    void LoadTexture(const StringView& texture_file);
    Texture* GetTexture(const StringView& texture_file)
    {
        for (size_t i = 0; i < textures.len; ++i) {
            if (textures[i]->name == texture_file) {
                return textures[i];
            }
        }
        return nullptr;
    }

    Model LoadModel(const StringView& model_file);
    void LoadShader(const StringView& shader_file);
    Shader* GetShader(const StringView& shader_file)
    {
        for (size_t i = 0; i < shaders.len; ++i) {
            if (shaders[i]->name == shader_file) {
                return shaders[i];
            }
        }
        return nullptr;
    }

    DISABLE_OBJECT_COPY_AND_MOVE(ResourceManager);
};
