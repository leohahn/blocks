#include "Importers/GLTF2.hpp"
#include "FileSystem.hpp"
#include "Json.hpp"
#include "Logger.hpp"
#include "ResourceManager.hpp"

#define TARGET_ELEMENT_ARRAY_BUFFER 0x8893
#define TARGET_ARRAY_BUFFER 0x8892

#define CHUNK_TYPE_JSON 0x4E4F534A
#define CHUNK_TYPE_BINARY 0x004E4942

struct GltfNode
{
    String name;
    int32_t mesh;
    Vec3 translation;
    Quaternion rotation;
};

struct TextureRef
{
    int32_t index = -1;
    int32_t tex_coord = -1;
};

struct GltfMaterial
{
    String name;
    bool double_sided = false;
    TextureRef base_color;
    TextureRef metallic_roughness;
};

struct GltfTexture
{
    int32_t source = -1;
    int32_t sampler = -1;
};

struct GltfImage
{
    String name;
    String mime_type;
    String uri;
};

struct GltfPrimitive
{
    RobinHashMap<String, int32_t> attributes;
    int32_t indices = -1;
    int32_t material = -1;
};

struct GltfMesh
{
    String name;
    Array<GltfPrimitive> primitives;
};

struct GltfBufferHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t length;

    static constexpr int kMagic = 0x46546C67;
};

struct GltfBufferChunk
{
    uint32_t length;
    uint32_t type;
    uint8_t* data;
};

static_assert(sizeof(GltfBufferHeader) == 12, "Should be 12 bytes large");

struct GltfBuffer
{
    int64_t byte_length = -1;
    String uri;
};

struct GltfBufferView
{
    int64_t buffer_index = -1;
    int64_t byte_length = -1;
    int64_t byte_offset = -1;
};

struct GltfAccessor
{
    String type;
    int64_t buffer_view_index = -1;
    int64_t component_type = -1;
    int64_t count = -1;
    Vec3 max = Vec3::Zero();
    Vec3 min = Vec3::Zero();
};

static bool
TryGetRotation(const Json::Val* rotation, Quaternion* out)
{
    assert(out);
    const Array<Json::Val>* rotation_array = rotation->AsArray();
    if (!rotation_array) {
        return false;
    }
    
    if (rotation_array->len != 4) {
        return false;
    }

    double x, y, z, w;

    if (!(*rotation_array)[0].TryConvertNumberToDouble(&x)) {
        return false;
    }
    if (!(*rotation_array)[1].TryConvertNumberToDouble(&y)) {
        return false;
    }
    if (!(*rotation_array)[2].TryConvertNumberToDouble(&z)) {
        return false;
    }
    if (!(*rotation_array)[3].TryConvertNumberToDouble(&w)) {
        return false;
    }
    
    *out = Quaternion(x, y, z, w);
    return true;
}
        
static bool
TryGetVec3(const Json::Val* vec, Vec3* out)
{
    assert(out);
    const Array<Json::Val>* translation_array = vec->AsArray();
    if (!translation_array) {
        return false;
    }
    
    if (translation_array->len != 3) {
        return false;
    }

    double x, y, z;

    if (!(*translation_array)[0].TryConvertNumberToDouble(&x)) {
        return false;
    }
    if (!(*translation_array)[1].TryConvertNumberToDouble(&y)) {
        return false;
    }
    if (!(*translation_array)[2].TryConvertNumberToDouble(&z)) {
        return false;
    }

    *out = Vec3(x, y, z);
    return true;
}

bool
TryGetNodes(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfNode>* out_nodes)
{
    assert(alloc);
    assert(out_nodes);
    
    const Json::Val* nodes_val = gltf_file->Find(String(alloc, "nodes"));
    if (!nodes_val) {
        LOG_ERROR("Was expecting a nodes array");
        return false;
    }
    
    const Array<Json::Val>* nodes = nodes_val->AsArray();
    if (!nodes) {
        LOG_ERROR("Was expecting a nodes array");
        return false;
    }
    
    *out_nodes = Array<GltfNode>(alloc);
    
    for (size_t i = 0; i < nodes->len; ++i) {
        const RobinHashMap<String, Json::Val>* raw_node = (*nodes)[i].AsObject();
        if (!raw_node) {
            LOG_ERROR("Was expecting a node object");
            return false;
        }
        
        const Json::Val* name_val = raw_node->Find(String(alloc, "name"));
        if (!name_val || !name_val->IsString()) {
            LOG_ERROR("Was expecting a name property");
            return false;
        }
        
        const Json::Val* mesh_val = raw_node->Find(String(alloc, "mesh"));
        if (!mesh_val || !mesh_val->IsInteger()) {
            LOG_ERROR("Was expecting a mesh property");
            return false;
        }

        const Json::Val* rotation_val = raw_node->Find(String(alloc, "rotation"));
        if (!rotation_val || !rotation_val->IsArray()) {
            LOG_ERROR("Was expecting a rotation property");
            return false;
        }

        const Json::Val* translation_val = raw_node->Find(String(alloc, "translation"));
        if (!translation_val || !translation_val->IsArray()) {
            LOG_ERROR("Was expecting a translation property");
            return false;
        }
        
        GltfNode out_node;
        out_node.name = String(alloc, name_val->AsString()->View());
        out_node.mesh = (int32_t)*mesh_val->AsInt64();

        if (!TryGetVec3(translation_val, &out_node.translation)) {
            LOG_ERROR("Failed to parse translation vector in node");
            return false;
        }

        if (!TryGetRotation(rotation_val, &out_node.rotation)) {
            LOG_ERROR("Failed to parse rotation vector in node");
            return false;
        }

        out_nodes->PushBack(std::move(out_node));
    }
    
    return true;
}

bool
TryGetMeshes(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfMesh>* out_meshes)
{
    assert(alloc);
    assert(out_meshes);
    
    const Json::Val* meshes_val = gltf_file->Find(String(alloc, "meshes"));
    if (!meshes_val) {
        LOG_ERROR("Was expecting a meshes array");
        return false;
    }
    
    const Array<Json::Val>* meshes = meshes_val->AsArray();
    if (!meshes) {
        LOG_ERROR("Was expecting a meshes array");
        return false;
    }
    
    *out_meshes = Array<GltfMesh>(alloc);
    
    for (size_t i = 0; i < meshes->len; ++i) {
        const RobinHashMap<String, Json::Val>* mesh = (*meshes)[i].AsObject();
        if (!mesh) {
            LOG_ERROR("Was expecting a mesh object");
            return false;
        }
        
        const Json::Val* name_val = mesh->Find(String(alloc, "name"));
        if (!name_val || !name_val->IsString()) {
            LOG_ERROR("Was expecting a name property");
            return false;
        }
        
        const Json::Val* primitives_val = mesh->Find(String(alloc, "primitives"));
        if (!primitives_val || !primitives_val->IsArray()) {
            LOG_ERROR("Was expecting a primitives array");
            return false;
        }
        
        GltfMesh out_mesh;
        out_mesh.name = String(alloc, name_val->AsString()->View());
        out_mesh.primitives = Array<GltfPrimitive>(alloc);
        
        for (size_t pi = 0; pi < primitives_val->AsArray()->len; ++pi) {
            const RobinHashMap<String, Json::Val>* raw_primitive = (*primitives_val->AsArray())[pi].AsObject();
            if (!raw_primitive) {
                LOG_ERROR("Was expecting a primitive object");
                assert(false);
            }

            const Json::Val* raw_indices = raw_primitive->Find(String(alloc, "indices"));
            if (!raw_indices || !raw_indices->IsInteger()) {
                LOG_ERROR("Was expecting a indice property");
                assert(false);
            }

            const Json::Val* raw_material = raw_primitive->Find(String(alloc, "material"));
            if (!raw_material || !raw_material->IsInteger()) {
                LOG_ERROR("Was expecting a material property");
                assert(false);
            }

            const Json::Val* raw_attributes = raw_primitive->Find(String(alloc, "attributes"));
            if (!raw_attributes || !raw_attributes->IsObject()) {
                LOG_ERROR("Was expecting an attributes property");
                assert(false);
            }


            GltfPrimitive primitive;
            primitive.indices = (int32_t)*raw_indices->AsInt64();
            primitive.material = (int32_t)*raw_material->AsInt64();
            primitive.attributes = RobinHashMap<String, int>(alloc, 16);

            for (const auto& pair : *raw_attributes->AsObject()) {
                const int64_t* val = pair.val.AsInt64();
                if (!val) {
                    LOG_ERROR("Was expecting integer as an attribute");
                    assert(false);
                }

                primitive.attributes.Add(
                    String(alloc, pair.key.View()),
                    (int32_t)*val
                );
            }

            out_mesh.primitives.PushBack(std::move(primitive));
        }
    }
    
    return true;
}

bool
TryGetBuffers(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfBuffer>* out_buffers)
{
    assert(alloc);
    assert(out_buffers);
    assert(gltf_file);
    
    const Json::Val* buffers_val = gltf_file->Find(String(alloc, "buffers"));
    if (!buffers_val) {
        LOG_ERROR("Was expecting a buffers array");
        return false;
    }
    
    const Array<Json::Val>* buffers = buffers_val->AsArray();
    if (!buffers) {
        LOG_ERROR("Was expecting a buffers array");
        return false;
    }
    
    *out_buffers = Array<GltfBuffer>(alloc);
    
    for (size_t i = 0; i < buffers->len; ++i) {
        const RobinHashMap<String, Json::Val>* buffer = (*buffers)[i].AsObject();
        if (!buffer) {
            LOG_ERROR("Was expecting a buffer object");
            return false;
        }
        
        const Json::Val* uri_val = buffer->Find(String(alloc, "uri"));
        if (!uri_val || !uri_val->IsString()) {
            LOG_ERROR("Was expecting a uri property");
            return false;
        }
        
        const Json::Val* byte_length_val = buffer->Find(String(alloc, "byteLength"));
        if (!byte_length_val || !byte_length_val->IsInteger()) {
            LOG_ERROR("Was expecting a byteLength property");
            return false;
        }
        
        GltfBuffer out_buf;
        out_buf.byte_length = *byte_length_val->AsInt64();
        out_buf.uri = String(alloc, uri_val->AsString()->View());
        out_buffers->PushBack(out_buf);
    }
    
    return true;
}

static bool
TryGetAccessors(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfAccessor>* out_accessors)
{
    assert(alloc);
    assert(gltf_file);
    assert(out_accessors);

    const Json::Val* accessors_val = gltf_file->Find(String(alloc, "accessors"));
    if (!accessors_val) {
        LOG_ERROR("Was expecting an accessors array");
        return false;
    }
    
    const Array<Json::Val>* accessors = accessors_val->AsArray();
    if (!accessors) {
        LOG_ERROR("Was expecting an accessors array");
        return false;
    }
    
    *out_accessors = Array<GltfAccessor>(alloc);
    
    for (size_t i = 0; i < accessors->len; ++i) {
        const RobinHashMap<String, Json::Val>* accessor = (*accessors)[i].AsObject();
        if (!accessor) {
            LOG_ERROR("Was expecting a buffer object");
            return false;
        }
        
        const Json::Val* buffer_view_val = accessor->Find(String(alloc, "bufferView"));
        if (!buffer_view_val || !buffer_view_val->IsInteger()) {
            LOG_ERROR("Was expecting a bufferView property");
            return false;
        }
        
        const Json::Val* component_type_val = accessor->Find(String(alloc, "componentType"));
        if (!component_type_val || !component_type_val->IsInteger()) {
            LOG_ERROR("Was expecting a componentType property");
            return false;
        }

        const Json::Val* count_val = accessor->Find(String(alloc, "count"));
        if (!count_val || !count_val->IsInteger()) {
            LOG_ERROR("Was expecting a componentType property");
            return false;
        }

        const Json::Val* max_val = accessor->Find(String(alloc, "max"));
        if (max_val && !max_val->IsArray()) {
            LOG_ERROR("Was expecting a max property");
            return false;
        }

        const Json::Val* min_val = accessor->Find(String(alloc, "min"));
        if (min_val && !min_val->IsArray()) {
            LOG_ERROR("Was expecting a min property");
            return false;
        }

        const Json::Val* type_val = accessor->Find(String(alloc, "type"));
        if (!type_val || !type_val->IsString()) {
            LOG_ERROR("Was expecting a type property");
            return false;
        }

        GltfAccessor out_accessor;
        out_accessor.buffer_view_index = *buffer_view_val->AsInt64();
        out_accessor.component_type = *component_type_val->AsInt64();
        out_accessor.count = *count_val->AsInt64();
        out_accessor.type = String(alloc, type_val->AsString()->View());

        if (max_val) {
            if (!TryGetVec3(max_val, &out_accessor.max)) {
                LOG_ERROR("Was expecting a max vector");
                return false;
            }
        } else {
            out_accessor.max = Vec3::Zero();
        }

        if (min_val) {
            if (!TryGetVec3(min_val, &out_accessor.min)) {
                LOG_ERROR("Was expecting a min vector");
                return false;
            }
        } else {
            out_accessor.min = Vec3::Zero();
        }

        out_accessors->PushBack(std::move(out_accessor));
    }
    
    return true;
}


static bool
TryGetMaterial(Allocator* alloc, const RobinHashMap<String, Json::Val>* raw_material, GltfMaterial* out_material)
{
    assert(alloc);
    assert(out_material);
    assert(raw_material);
    
    const Json::Val* material_name = raw_material->Find(String(alloc, "name"));
    if (!material_name || !material_name->AsString()) {
        LOG_ERROR("Was expecting material name");
        return false;
    }

    const Json::Val* double_sided_val = raw_material->Find(String(alloc, "doubleSided"));
    if (!double_sided_val || !double_sided_val->AsBool()) {
        LOG_ERROR("Was expecting a doubleSided property");
        return false;
    }
    
    const Json::Val* pbr_params = raw_material->Find(String(alloc, "pbrMetallicRoughness"));
    if (!pbr_params || !pbr_params->AsObject()) {
        LOG_ERROR("Was expecting pbr metallic roughness");
        return false;
    }
    
    const Json::Val* base_color_texture = pbr_params->AsObject()->Find(String(alloc, "baseColorTexture"));
    if (!base_color_texture || !base_color_texture->IsObject()) {
        LOG_ERROR("Was expecting baseColorTexture");
        return false;
    }

    const Json::Val* metallic_roughness_texture = pbr_params->AsObject()->Find(String(alloc, "metallicRoughnessTexture"));
    if (!metallic_roughness_texture || !metallic_roughness_texture->IsObject()) {
        LOG_ERROR("Was expecting metallicRoughnessTexture");
        return false;
    }

    const Json::Val* base_color_texture_index = base_color_texture->AsObject()->Find(String(alloc, "index"));
    if (!base_color_texture_index || !base_color_texture_index->IsInteger()) {
        LOG_ERROR("Was expecting base texture index");
        return false;
    }

    const Json::Val* base_color_texture_tex_coord = base_color_texture->AsObject()->Find(String(alloc, "texCoord"));
    if (!base_color_texture_tex_coord || !base_color_texture_tex_coord->IsInteger()) {
        LOG_ERROR("Was expecting base texture texCoord");
        return false;
    }

    const Json::Val* metallic_roughness_texture_index = metallic_roughness_texture->AsObject()->Find(String(alloc, "index"));
    if (!metallic_roughness_texture_index || !metallic_roughness_texture_index->IsInteger()) {
        LOG_ERROR("Was expecting metallic roughness texture index");
        return false;
    }

    const Json::Val* metallic_roughness_texture_tex_coord = metallic_roughness_texture->AsObject()->Find(String(alloc, "texCoord"));
    if (!metallic_roughness_texture_tex_coord || !metallic_roughness_texture_tex_coord->IsInteger()) {
        LOG_ERROR("Was expecting metallic roughness texture texCoord");
        return false;
    }

    GltfMaterial out_mat;
    out_mat.name = String(alloc, material_name->AsString()->View());
    out_mat.double_sided = *double_sided_val->AsBool();
    out_mat.base_color.index = *base_color_texture_index->AsInt64();
    out_mat.base_color.tex_coord = *base_color_texture_tex_coord->AsInt64();
    out_mat.metallic_roughness.index = *metallic_roughness_texture_index->AsInt64();
    out_mat.metallic_roughness.tex_coord = *metallic_roughness_texture_tex_coord->AsInt64();

    *out_material = std::move(out_mat);
    return true;
}

static bool
TryGetMaterials(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfMaterial>* out_materials)
{
    assert(gltf_file);
    assert(alloc);
    assert(out_materials);

    const Json::Val* materials_val = gltf_file->Find(String(alloc, "materials"));
    if (!materials_val) {
        LOG_ERROR("Was expecting a materials array");
        return false;
    }
    
    const Array<Json::Val>* materials = materials_val->AsArray();
    if (!materials) {
        LOG_ERROR("Was expecting a materials array");
        return false;
    }

    *out_materials = Array<GltfMaterial>(alloc);

    for (size_t mi = 0; mi < materials->len; ++mi) {
        const RobinHashMap<String, Json::Val>* raw_material = (*materials)[mi].AsObject();
        if (!raw_material) {
            LOG_ERROR("Was expecting a material object");
            return false;
        }

        GltfMaterial material;
        if (!TryGetMaterial(alloc, raw_material, &material)) {
            LOG_ERROR("Was expecting a material object");
            return false;
        }

        out_materials->PushBack(std::move(material));
    }
    
    return true;
}

static bool
TryGetImages(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfImage>* out_images)
{
    assert(gltf_file);
    assert(alloc);
    assert(out_images);

    const Json::Val* images_val = gltf_file->Find(String(alloc, "images"));
    if (!images_val) {
        LOG_ERROR("Was expecting a materials array");
        return false;
    }
    
    const Array<Json::Val>* images = images_val->AsArray();
    if (!images) {
        LOG_ERROR("Was expecting a materials array");
        return false;
    }

    *out_images = Array<GltfImage>(alloc);

    for (size_t mi = 0; mi < images->len; ++mi) {
        const RobinHashMap<String, Json::Val>* raw_image = (*images)[mi].AsObject();
        if (!raw_image) {
            LOG_ERROR("Was expecting an image object");
            return false;
        }

        const Json::Val* mime_type_val = raw_image->Find(String(alloc, "mimeType"));
        if (!mime_type_val || !mime_type_val->IsString()) {
            LOG_ERROR("Was expecting a mimeType property");
            return false;
        }

        const Json::Val* name_val = raw_image->Find(String(alloc, "name"));
        if (!name_val || !name_val->IsString()) {
            LOG_ERROR("Was expecting a name property");
            return false;
        }

        const Json::Val* uri_val = raw_image->Find(String(alloc, "uri"));
        if (!uri_val || !uri_val->IsString()) {
            LOG_ERROR("Was expecting a uri property");
            return false;
        }

        GltfImage out_image;
        out_image.mime_type = String(alloc, mime_type_val->AsString()->View());
        out_image.name = String(alloc, name_val->AsString()->View());
        out_image.uri = String(alloc, uri_val->AsString()->View());

        out_images->PushBack(std::move(out_image));
    }
    
    return true;
}

static bool
TryGetBufferViews(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfBufferView>* out_buffer_views)
{
    assert(gltf_file);
    assert(alloc);
    assert(out_buffer_views);

    const Json::Val* buffer_views_val = gltf_file->Find(String(alloc, "bufferViews"));
    if (!buffer_views_val) {
        LOG_ERROR("Was expecting a bufferViews array");
        return false;
    }
    
    const Array<Json::Val>* buffer_views = buffer_views_val->AsArray();
    if (!buffer_views) {
        LOG_ERROR("Was expecting a materials array");
        return false;
    }

    *out_buffer_views = Array<GltfBufferView>(alloc);

    for (size_t mi = 0; mi < buffer_views->len; ++mi) {
        const RobinHashMap<String, Json::Val>* raw_buffer_view = (*buffer_views)[mi].AsObject();
        if (!raw_buffer_view) {
            LOG_ERROR("Was expecting a bufferView object");
            return false;
        }

        const Json::Val* byte_length_val = raw_buffer_view->Find(String(alloc, "byteLength"));
        if (!byte_length_val || !byte_length_val->IsInteger()) {
            LOG_ERROR("Was expecting a byteLength property");
            return false;
        }

        const Json::Val* buffer_val = raw_buffer_view->Find(String(alloc, "buffer"));
        if (!buffer_val || !buffer_val->IsInteger()) {
            LOG_ERROR("Was expecting a buffer property");
            return false;
        }

        const Json::Val* byte_offset_val = raw_buffer_view->Find(String(alloc, "byteOffset"));
        if (!byte_offset_val || !byte_offset_val->IsInteger()) {
            LOG_ERROR("Was expecting a byteOffset property");
            return false;
        }

        GltfBufferView out_buffer_view;
        out_buffer_view.buffer_index = *buffer_val->AsInt64();
        out_buffer_view.byte_length = *byte_length_val->AsInt64();
        out_buffer_view.byte_offset = *byte_offset_val->AsInt64();

        out_buffer_views->PushBack(std::move(out_buffer_view));
    }
    
    return true;
}

static bool
TryGetTextures(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfTexture>* out_textures)
{
    assert(alloc);
    assert(gltf_file);
    assert(out_textures);

    const Json::Val* textures_val = gltf_file->Find(String(alloc, "textures"));
    if (!textures_val) {
        LOG_ERROR("Was expecting a textures array");
        return false;
    }
    
    const Array<Json::Val>* textures = textures_val->AsArray();
    if (!textures) {
        LOG_ERROR("Was expecting a textures array");
        return false;
    }

    *out_textures = Array<GltfTexture>(alloc);

    for (size_t mi = 0; mi < textures->len; ++mi) {
        const RobinHashMap<String, Json::Val>* raw_texture = (*textures)[mi].AsObject();
        if (!raw_texture) {
            LOG_ERROR("Was expecting a texture object");
            return false;
        }

        const Json::Val* source_val = raw_texture->Find(String(alloc, "source"));
        if (!source_val || !source_val->IsInteger()) {
            LOG_ERROR("Was expecting a source property");
            return false;
        }

        const Json::Val* sampler_val = raw_texture->Find(String(alloc, "sampler"));
        if (sampler_val || !sampler_val->IsInteger()) {
            LOG_ERROR("Was expecting a sampler property");
            return false;
        }

        GltfTexture out_texture;
        out_texture.source = (int32_t)*source_val->AsInt64();
        if (sampler_val) {
            out_texture.sampler = (int32_t)*sampler_val->AsInt64();
        } else {
            out_texture.sampler = 0;
        }

        out_textures->PushBack(std::move(out_texture));
    }
    
    return true;
}

struct LoadedGltfBuffer
{
    Allocator* allocator;
    uint8_t* buffer_data;
    Array<GltfBufferChunk*> chunks;

    LoadedGltfBuffer()
        : allocator(nullptr)
        , buffer_data(nullptr)
        , chunks(nullptr)
    {}

    LoadedGltfBuffer(Allocator* allocator, uint8_t* buffer_data)
        : allocator(allocator)
        , buffer_data(buffer_data)
        , chunks(allocator)
    {
    }

    ~LoadedGltfBuffer()
    {
        assert(allocator);
        allocator->Deallocate(buffer_data);
    }

    LoadedGltfBuffer(LoadedGltfBuffer&& buf)
        : LoadedGltfBuffer()
    {
        *this = std::move(buf);
    }

    LoadedGltfBuffer& operator=(LoadedGltfBuffer&& buf)
    {
        if (buffer_data && allocator) {
            allocator->Deallocate(buffer_data);
        }
        allocator = buf.allocator;
        buffer_data = buf.buffer_data;
        chunks = std::move(buf.chunks);
        buf.allocator = nullptr;
        buf.buffer_data = nullptr;
        return *this;
    }

    // delete copy constructors
    LoadedGltfBuffer(const LoadedGltfBuffer&) = delete;
    LoadedGltfBuffer& operator=(const LoadedGltfBuffer&) = delete;
};

LoadedGltfBuffer
LoadGltfBuffer(Allocator* alloc, const GltfBuffer& buffer)
{
    Path file_path = FileSystem::GetResourcesPath(alloc);
    file_path.Push(buffer.uri.View());

    size_t buffer_size;
    uint8_t* buffer_data = FileSystem::LoadFileToMemory(alloc, file_path, &buffer_size);
    assert(buffer_data);

    GltfBufferHeader* header = (GltfBufferHeader*)buffer_data;
    assert(header->magic == GltfBufferHeader::kMagic);
    assert(header->version == 2);

    LOG_DEBUG("Binary buffer is %lu bytes", header->length);

    // start reading the chunks
    const uint32_t chunks_total_size = header->length - sizeof(GltfBufferHeader);
    uint32_t offset = sizeof(GltfBufferHeader);

    LoadedGltfBuffer loaded_buffer(alloc, buffer_data);

    for (;;) {
        const uint32_t bytes_left = header->length - offset;
        if (bytes_left < sizeof(GltfBufferChunk) + 1) {
            break;
        }

        GltfBufferChunk* chunk = (GltfBufferChunk*)(buffer_data + offset);
        assert(chunk->type == CHUNK_TYPE_BINARY && "Json chunk is not supported");
        loaded_buffer.chunks.PushBack(chunk);

        offset += chunk->length;
    }

    return loaded_buffer;
}

Model
ImportGltf2Model(Allocator* alloc, Allocator* scratch_allocator, const Path& path, ResourceManager* resource_manager, int model_index)
{
    size_t size;
    uint8_t* data = FileSystem::LoadFileToMemory(scratch_allocator, path, &size);
    assert(data);
    
    Model model(alloc);

    Json::Document doc(scratch_allocator);
    doc.Parse(data, size);
    if (doc.HasParseErrors() || !doc.root_val.IsObject()) {
        LOG_ERROR("GLTF2 file is corrupt: %s", doc.GetErrorStr());
        assert(false);
    }

    assert(doc.root_val.type == Json::Type::Object);
    
    const RobinHashMap<String, Json::Val>* root = doc.root_val.AsObject();
    if (!root) {
        LOG_ERROR("Was expecting root to be an object");
        assert(false);
    }

    Array<GltfBuffer> buffers;
    if (!TryGetBuffers(alloc, root, &buffers)) {
        LOG_ERROR("Was expecting a buffers array");
        assert(false);
    }

    Array<GltfMesh> meshes;
    if (!TryGetMeshes(alloc, root, &meshes)) {
        LOG_ERROR("Was expecting a meshes array");
        assert(false);
    }

    Array<GltfNode> nodes;
    if (!TryGetNodes(alloc, root, &nodes)) {
        LOG_ERROR("Was expecting a nodes array");
        assert(false);
    }

    Array<GltfMaterial> materials;
    if (!TryGetMaterials(alloc, root, &materials)) {
        LOG_ERROR("Was expecting a materials array");
        assert(false);
    }

    Array<GltfImage> images;
    if (!TryGetImages(alloc, root, &images)) {
        LOG_ERROR("Was expecting an images array");
        assert(false);
    }

    Array<GltfAccessor> accessors;
    if (!TryGetAccessors(alloc, root, &accessors)) {
        LOG_ERROR("Was expecting an accessors array");
        assert(false);
    }

    Array<GltfBufferView> buffer_views;
    if (!TryGetBufferViews(alloc, root, &buffer_views)) {
        LOG_ERROR("Was expecting a bufferViews array");
        assert(false);
    }

    Array<GltfTexture> textures;
    if (!TryGetTextures(alloc, root, &textures)) {
        LOG_ERROR("Was expecting a bufferViews array");
        assert(false);
    }

    // Load the buffers
    Array<LoadedGltfBuffer> loaded_buffers(scratch_allocator);

    for (size_t bi = 0; bi < buffers.len; ++bi) {
        const GltfBuffer& buffer = buffers[bi];
        LoadedGltfBuffer loaded_buffer = LoadGltfBuffer(scratch_allocator, buffer);
        loaded_buffers.PushBack(std::move(loaded_buffer));
    }

    // A node inside gltf will be represented as a model.
    assert(nodes.len == 1 && "only one node supported currently");

    const GltfNode& node = nodes[0];

    Model model(alloc);
    model.name = SID(node.name.data);
    model.rotation = node.rotation;
    model.translation = node.translation;
    model.scale = 1.0f;
    model.meshes = Array<TriangleMesh*>(alloc);

    const GltfMesh& gltf_mesh = meshes[node.mesh];

    // Load all textures
    for (size_t ti = 0; ti < textures.len; ++ti) {
        const GltfTexture& gltf_texture = textures[ti];
        const GltfImage& gltf_image = images[gltf_texture.source];
        resource_manager->LoadTexture(SID(gltf_image.uri.data));
    }

    // Create all materials
    for (size_t mi = 0; mi < materials.len; ++mi) {
        const GltfMaterial& gltf_material = materials[mi];
        // TODO: include metallic roughness texture

        Material* material = resource_manager->allocator->New<Material>(resource_manager->allocator);
        material->name = SID(gltf_material.name.data);

        if (gltf_material.base_color.index > -1) {
            const GltfImage& gltf_base_image = images[gltf_material.base_color.index];
            material->diffuse_map = resource_manager->GetTexture(SID(gltf_base_image.uri.data));
        }

        resource_manager->materials.Add(material->name, material);
    }

    // start loading the triangle mesh
    auto mesh = resource_manager->allocator->New<TriangleMesh>(resource_manager->allocator);
    mesh->name = SID(gltf_mesh.name.data);
    mesh->sub_meshes = Array<SubMesh>(resource_manager->allocator);

    for (size_t pi = 0; pi < gltf_mesh.primitives.len; ++pi) {
        const GltfPrimitive& primitive = gltf_mesh.primitives[pi];
        const GltfMaterial& material = materials[primitive.material];
        const GltfAccessor& indices_accessor = accessors[primitive.indices];
        const GltfBufferView& indices_buffer_view = buffer_views[indices_accessor.buffer_view_index];

        // Each primitive is a submesh in the engine currently.
        // TODO: improve how nodes are represented in the engine
        SubMesh submesh;
        submesh.material = resource_manager->GetMaterial(SID(material.name.data));
        submesh.start_index = indices_buffer_view.byte_offset;
        submesh.num_indices = indices_buffer_view.byte_length;

        assert(submesh.material);

        for (const auto& pair : primitive.attributes) {
            const int32_t accessor_index = pair.val;
            const GltfAccessor& accessor = accessors[accessor_index];
            const GltfBufferView& buffer_view = buffer_views[accessor.buffer_view_index];

            if (pair.key == "POSITION") {
            } else if (pair.key == "NORMAL") {

            } else if (pair.key == "TEXCOORD_0") {

            } else {
                LOG_ERROR("Unknown attribute %s", pair.key.data);
                assert(false);
            }
        }

        mesh->sub_meshes.PushBack(std::move(submesh));
    }

    //
    // TODO: Now we can start loading the node/model into memory
    //

    scratch_allocator->Deallocate(data);
    return Model(alloc);
}
