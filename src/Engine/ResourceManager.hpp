#pragma once

#include "Han/Allocator.hpp"
#include "Han/Collections/Array.hpp"
#include "Han/Core.hpp"
#include "Han/Math/Quaternion.hpp"
#include "Han/Sid.hpp"
#include "Path.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "TriangleMesh.hpp"
#include "ResourceFile.hpp"
#include "Model.hpp"

enum LoadTextureFlags
{
    LoadTextureFlags_None = 0,
    LoadTextureFlags_FlipVertically = BIT(0),
    LoadTextureFlags_LinearSpace = BIT(1),
};

struct ResourceManager
{
    Allocator* allocator;
    Allocator* scratch_allocator;
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
        , resources_path(allocator)
        , textures(allocator, kNumTextures)
        , shaders(allocator, kNumShaders)
        , meshes(allocator, kNumMeshes)
        , materials(allocator, kNumMaterials)
    {}

    ResourceManager(ResourceManager&& other) = default;

    ResourceManager& operator=(ResourceManager&& other) = default;

    void Create();
    void Destroy();

    Texture* LoadTexture(const Sid& texture_file, int flags = LoadTextureFlags_None);
    Texture* GetTexture(const Sid& texture_file)
    {
        return *textures.Find(texture_file);
    }

    Material* GetMaterial(const Sid& material_name)
    {
        return *materials.Find(material_name);
    }

    Model LoadModel(const Sid& model_file);

    Model LoadObjModel(const ResourceFile& res_file);
    Model LoadGltfModel(const ResourceFile& res_file);

    void LoadShader(const Sid& shader_file);
    Shader* GetShader(const Sid& shader_file)
    {
        return *shaders.Find(shader_file);
    }

    DISABLE_OBJECT_COPY(ResourceManager);
};
