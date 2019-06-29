#pragma once

#include "Allocator.hpp"
#include "Collections/Array.hpp"
#include "Defines.hpp"
#include "Math/Quaternion.hpp"
#include "Path.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "TriangleMesh.hpp"
#include "Sid.hpp"

struct Model
{
    // TODO: transform a model into two different classes:
    // a model and a model instance. A model instance will be a light weight model
    // with orientation, scale, etc.
    Array<TriangleMesh*> meshes;
    Vec3 position;
    Quaternion orientation;
    float scale;

public:
    Model(Allocator* allocator)
        : meshes(allocator)
    {}
    
    Model(Model&& model) = default;

    DISABLE_OBJECT_COPY(Model);
};

struct ResourceManager
{
    Allocator* allocator;
    Allocator* scratch_allocator;
    //Array<TriangleMesh*> meshes;
    //Array<Texture*> textures;
    //Array<Shader*> shaders;
    Path resources_path;

    RobinHashMap<Sid, Texture*> textures;
    RobinHashMap<Sid, Shader*> shaders;
    RobinHashMap<Sid, TriangleMesh*> meshes;
    RobinHashMap<Sid, Material*> materials;

public:
    static constexpr int kNumMeshes = 32;
    static constexpr int kNumTextures = 32;
    static constexpr int kNumShaders = 32;
    static constexpr int kNumMaterials = 32;

    ResourceManager(Allocator* allocator, Allocator* scratch_allocator)
        : allocator(allocator)
        , scratch_allocator(scratch_allocator)
        , meshes(allocator, kNumMeshes)
        , materials(allocator, kNumMaterials)
        , textures(allocator, kNumTextures)
        , shaders(allocator, kNumShaders)
        , resources_path(allocator)
    {}

    ResourceManager(ResourceManager&& other) = default;

    ResourceManager& operator=(ResourceManager&& other) = default;

    void Create();
    void Destroy();

    void LoadTexture(const Sid& texture_file);
    Texture* GetTexture(const Sid& texture_file)
    {
        return *textures.Find(texture_file);
    }

    Material* GetMaterial(const Sid& material_name)
    {
        return *materials.Find(material_name);
    }

    Model LoadModel(const Sid& model_file);
    void LoadShader(const Sid& shader_file);
    Shader* GetShader(const Sid& shader_file)
    {
        return *shaders.Find(shader_file);
    }

    DISABLE_OBJECT_COPY(ResourceManager);
};
