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
    TextureRef normal;
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

struct GlbBufferHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t length;

    static constexpr int kMagic = 0x46546C67;
};

struct GlbBufferChunk
{
    uint32_t length;
    uint32_t type;
    uint8_t* data;
};

static_assert(sizeof(GlbBufferHeader) == 12, "Should be 12 bytes large");

struct GltfBuffer
{
    Allocator* allocator;
    int64_t byte_length = -1;
    String uri;
    uint8_t* data;

    GltfBuffer(Allocator* alloc, const StringView& uri, int64_t byte_length)
        : allocator(alloc)
        , uri(alloc, uri)
        , byte_length(byte_length)
    {
        Path path = FileSystem::GetResourcesPath(alloc);
        path.Push(uri);

        size_t file_size;
        data = FileSystem::LoadFileToMemory(alloc, path, &file_size);

        ASSERT(file_size == byte_length, "sizes should be equal");
        ASSERT(data, "The buffer should exist");
    }

    GltfBuffer(GltfBuffer&& buf)
        : data(nullptr)
    {
        *this = std::move(buf);
    }

    GltfBuffer& operator=(GltfBuffer&& buf)
    {
        if (data) {
            allocator->Deallocate(data);
        }
        allocator = buf.allocator;
        uri = std::move(buf.uri);
        byte_length = buf.byte_length;
        data = buf.data;
        buf.allocator = nullptr;
        buf.data = nullptr;
        buf.byte_length = 0;
        return *this;
    }

    GltfBuffer& operator=(const GltfBuffer&) = delete;
    GltfBuffer(const GltfBuffer&) = delete;
};

enum class GltfBufferViewTarget
{
    Undefined = 0,
    ArrayBuffer = 34962,
    ElementArrayBuffer = 34963,
};

struct GltfBufferView
{
    GltfBufferViewTarget target = GltfBufferViewTarget::Undefined;
    int64_t buffer_index = -1;
    int64_t byte_length = -1;
    int64_t byte_offset = -1;
};

enum class ComponentType
{
    Byte = 5120,
    UnsignedByte = 5121,
    Short = 5122,
    UnsignedShort = 5123,
    UnsignedInt = 5125,
    Float = 5126,
};

static bool
TryGetComponentType(int64_t component_type, ComponentType* out_component_type)
{
    ASSERT(out_component_type, "should be a valid pointer");
    if (component_type == static_cast<int64_t>(ComponentType::Byte)) {
        *out_component_type = ComponentType::Byte;
    } else if (component_type == static_cast<int64_t>(ComponentType::UnsignedByte)) {
        *out_component_type = ComponentType::UnsignedByte;
    } else if (component_type == static_cast<int64_t>(ComponentType::Short)) {
        *out_component_type = ComponentType::Short;
    } else if (component_type == static_cast<int64_t>(ComponentType::UnsignedShort)) {
        *out_component_type = ComponentType::UnsignedShort;
    } else if (component_type == static_cast<int64_t>(ComponentType::UnsignedInt)) {
        *out_component_type = ComponentType::UnsignedInt;
    } else if (component_type == static_cast<int64_t>(ComponentType::Float)) {
        *out_component_type = ComponentType::Float;
    } else {
        return false;
    }
    return true;
}

enum class AccessorType
{
    Scalar = 1,
    Vec2 = 2,
    Vec3 = 3,
    Vec4 = 4,
    Mat2 = 4,
    Mat3 = 9,
    Mat4 = 16,
};

static bool
TryGetAccessorType(const StringView& str, AccessorType* out_type)
{
    ASSERT(out_type, "should be a valid pointer");
    if (str == "SCALAR") {
        *out_type = AccessorType::Scalar;
    } else if (str == "VEC2") {
        *out_type = AccessorType::Vec2;
    } else if (str == "VEC3") {
        *out_type = AccessorType::Vec3;
    } else if (str == "VEC4") {
        *out_type = AccessorType::Vec4;
    } else if (str == "MAT2") {
        *out_type = AccessorType::Mat2;
    } else if (str == "MAT3") {
        *out_type = AccessorType::Mat3;
    } else if (str == "MAT4") {
        *out_type = AccessorType::Mat4;
    } else {
        return false;
    }
    return true;
}

struct GltfAccessor
{
    AccessorType type = AccessorType::Vec3;
    ComponentType component_type = ComponentType::Float;
    int64_t buffer_view_index = -1;
    int64_t count = -1;
    Vec3 max = Vec3::Zero();
    Vec3 min = Vec3::Zero();
    uint32_t opengl_buffer = 0;
    bool normalized = false;

    int64_t GetElementSize() const
    {
        int64_t num_comp = static_cast<int64_t>(type);
        switch (component_type) {
            case ComponentType::Byte:          return num_comp * 1;
            case ComponentType::UnsignedByte:  return num_comp * 1;
            case ComponentType::Short:         return num_comp * 2;
            case ComponentType::UnsignedShort: return num_comp * 2;
            case ComponentType::UnsignedInt:   return num_comp * 4;
            case ComponentType::Float:         return num_comp * 4;
            default: ASSERT(false, "Unknown component type");
        }
    }
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
    
    *out = Quaternion((float)x, (float)y, (float)z, (float)w);
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

        out_meshes->PushBack(std::move(out_mesh));
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

        GltfBuffer out_buf(alloc, uri_val->AsString()->View(), *byte_length_val->AsInt64());

        out_buffers->PushBack(std::move(out_buf));
    }
    
    return true;
}

static bool
TryGetAccessors(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file,
                const Array<GltfBufferView>& buffer_views, Array<GltfAccessor>* out_accessors)
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

        const Json::Val* normalized_val = accessor->Find(String(alloc, "normalized"));
        if (normalized_val && !normalized_val->IsBool()) {
            LOG_ERROR("Was expecting a normalized property");
            return false;
        }

        GltfAccessor out_accessor;
        out_accessor.buffer_view_index = *buffer_view_val->AsInt64();
        out_accessor.count = *count_val->AsInt64();
        out_accessor.normalized = normalized_val ? *normalized_val->AsBool() : false;

        if (out_accessor.buffer_view_index >= buffer_views.len) {
            LOG_ERROR("Invalid buffer view index in accessor: %d", out_accessor.buffer_view_index);
            return false;
        }

        if (!TryGetAccessorType(type_val->AsString()->View(), &out_accessor.type)) {
            LOG_ERROR("Invalid accessor type %s", type_val->AsString()->data);
            return false;
        }

        if (!TryGetComponentType(*component_type_val->AsInt64(), &out_accessor.component_type)) {
            LOG_ERROR("Invalid component type %s", *component_type_val->AsInt64());
            return false;
        }

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

    const Json::Val* normal_texture = raw_material->Find(String(alloc, "normalTexture"));
    if (!normal_texture || !normal_texture->IsObject()) {
        LOG_ERROR("Was expecting normalTexture");
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

    const Json::Val* normal_texture_index = normal_texture->AsObject()->Find(String(alloc, "index"));
    if (!normal_texture_index || !normal_texture_index->IsInteger()) {
        LOG_ERROR("Was expecting normal texture index");
        return false;
    }

    const Json::Val* normal_texture_tex_coord = normal_texture->AsObject()->Find(String(alloc, "texCoord"));
    if (!normal_texture_tex_coord || !normal_texture_tex_coord->IsInteger()) {
        LOG_ERROR("Was expecting normal texture texCoord");
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
    out_mat.base_color.index = (int32_t)*base_color_texture_index->AsInt64();
    out_mat.base_color.tex_coord = (int32_t)*base_color_texture_tex_coord->AsInt64();
    out_mat.metallic_roughness.index = (int32_t)*metallic_roughness_texture_index->AsInt64();
    out_mat.metallic_roughness.tex_coord = (int32_t)*metallic_roughness_texture_tex_coord->AsInt64();
    out_mat.normal.index = (int32_t)*normal_texture_index->AsInt64();
    out_mat.normal.tex_coord = (int32_t)*normal_texture_tex_coord->AsInt64();
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

        const Json::Val* target_val = raw_buffer_view->Find(String(alloc, "target"));
        if (target_val && !target_val->IsInteger()) {
            LOG_ERROR("Was expecting a target property");
            return false;
        }

        GltfBufferView out_buffer_view;
        out_buffer_view.buffer_index = *buffer_val->AsInt64();
        out_buffer_view.byte_length = *byte_length_val->AsInt64();
        out_buffer_view.byte_offset = *byte_offset_val->AsInt64();

        if (target_val) {
            int64_t target = *target_val->AsInt64();
            if (target != static_cast<int64_t>(GltfBufferViewTarget::ArrayBuffer) ||
                target != static_cast<int64_t>(GltfBufferViewTarget::ElementArrayBuffer))
            {
                LOG_ERROR("Invalid buffer view target");
                return false;
            }
            out_buffer_view.target = static_cast<GltfBufferViewTarget>(target);
        }

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
        if (sampler_val && !sampler_val->IsInteger()) {
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

#if 0
struct LoadedGltfBuffer
{
    Allocator* allocator;
    uint8_t* buffer_data;
    Array<GltfBufferChunk*> chunks;
    uint32_t vertex_buffer;

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

//LoadedGltfBuffer
//LoadGltfBuffer(Allocator* alloc, const GltfBuffer& buffer)
//{
    //Path file_path = FileSystem::GetResourcesPath(alloc);
    //file_path.Push(buffer.uri.View());
//
    //size_t buffer_size;
    //uint8_t* buffer_data = FileSystem::LoadFileToMemory(alloc, file_path, &buffer_size);
    //assert(buffer_data);
//
    //GltfBufferHeader* header = (GltfBufferHeader*)buffer_data;
    //assert(header->magic == GltfBufferHeader::kMagic);
    //assert(header->version == 2);
//
    //LOG_DEBUG("Binary buffer is %lu bytes", header->length);
//
    //// start reading the chunks
    //const uint32_t chunks_total_size = header->length - sizeof(GltfBufferHeader);
    //uint32_t offset = sizeof(GltfBufferHeader);
//
    //LoadedGltfBuffer loaded_buffer(alloc, buffer_data);
//
    //for (;;) {
        //const uint32_t bytes_left = header->length - offset;
        //if (bytes_left < sizeof(GltfBufferChunk) + 1) {
            //break;
        //}
//
        //GltfBufferChunk* chunk = (GltfBufferChunk*)(buffer_data + offset);
        //assert(chunk->type == CHUNK_TYPE_BINARY && "Json chunk is not supported");
        //loaded_buffer.chunks.PushBack(chunk);
//
        //offset += chunk->length;
    //}
//
    //return loaded_buffer;
//}
#endif

Model
ImportGltf2Model(Allocator* alloc, Allocator* scratch_allocator, const Path& path, ResourceManager* resource_manager, int model_index)
{
    size_t size;
    uint8_t* data = FileSystem::LoadFileToMemory(scratch_allocator, path, &size);
    assert(data);
    
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

    Array<GltfBufferView> buffer_views;
    if (!TryGetBufferViews(alloc, root, &buffer_views)) {
        LOG_ERROR("Was expecting a bufferViews array");
        assert(false);
    }

    Array<GltfAccessor> accessors;
    if (!TryGetAccessors(alloc, root, buffer_views, &accessors)) {
        LOG_ERROR("Was expecting an accessors array");
        assert(false);
    }

    Array<GltfTexture> textures;
    if (!TryGetTextures(alloc, root, &textures)) {
        ASSERT(false, "Was expecting a bufferViews array");
    }

    // A node inside gltf will be represented as a model.
    ASSERT(nodes.len == 1, "only one node supported currently");

    const GltfNode& node = nodes[0];

    Model model(alloc);
    model.name = SID(node.name.data);
    model.rotation = node.rotation;
    model.translation = node.translation;
    model.scale = 1.0f;

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
        material->shader = resource_manager->GetShader(SID("pbr.glsl"));
        ASSERT(material->shader, "shader is not loaded!");

        if (gltf_material.base_color.index > -1) {
            const GltfImage& gltf_base_image = images[gltf_material.base_color.index];
            material->AddValue(SID("u_albedo_texture"), resource_manager->GetTexture(SID(gltf_base_image.uri.data)));
        }

        if (gltf_material.metallic_roughness.index > -1) {
            const GltfImage& gltf_base_image = images[gltf_material.metallic_roughness.index];
            material->AddValue(SID("u_metallic_roughness_texture"), resource_manager->GetTexture(SID(gltf_base_image.uri.data)));
        }

        if (gltf_material.normal.index > -1) {
            const GltfImage& gltf_base_image = images[gltf_material.normal.index];
            material->AddValue(SID("u_normal_texture"), resource_manager->GetTexture(SID(gltf_base_image.uri.data)));
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

        IndexBuffer* ibo = nullptr;

        if (indices_accessor.opengl_buffer == 0) {
            const GltfBufferView& buffer_view = buffer_views[indices_accessor.buffer_view_index];
            const GltfBuffer& buffer = buffers[buffer_view.buffer_index];
            size_t buffer_size = indices_accessor.GetElementSize() * indices_accessor.count;
            ASSERT(buffer_view.byte_length >= buffer_size, "Buffer view is too small!");

            ASSERT(indices_accessor.type == AccessorType::Scalar, "should be a scalar");
            if (indices_accessor.component_type == ComponentType::UnsignedInt) {
                ibo = IndexBuffer::Create(alloc, (uint32_t*)(buffer.data + buffer_view.byte_offset), indices_accessor.count);
            } else if (indices_accessor.component_type == ComponentType::UnsignedShort) {
                ibo = IndexBuffer::Create(alloc, (uint16_t*)(buffer.data + buffer_view.byte_offset), indices_accessor.count);
            } else if (indices_accessor.component_type == ComponentType::UnsignedByte) {
                // TODO: add support for indices of unsigned byte
                //ibo = IndexBuffer::Create(alloc, (uint8_t*)(buffer.data + buffer_view.byte_offset), indices_accessor.count);
                ASSERT(false, "Unsupported component type");
            } else {
                ASSERT(false, "Unsupported component type");
            }
        }

        LOG_DEBUG("Gltf2 model is using an index buffer for mesh %s", gltf_mesh.name.data);

        // Each primitive is a submesh in the engine currently.
        // TODO: improve how nodes are represented in the engine
        SubMesh submesh;
        submesh.material = resource_manager->GetMaterial(SID(material.name.data));
        submesh.start_index = 0;
        submesh.num_indices = indices_accessor.count;
        ASSERT(submesh.material, "material should exist");

        //
        // We will combine the primitive buffer views into one buffer in order to send it to the GPU 
        //
        const int32_t* position_accessor_index = primitive.attributes.Find(String(scratch_allocator, "POSITION"));
        ASSERT(position_accessor_index, "should have a position accessor");
        const GltfAccessor& position_accessor = accessors[*position_accessor_index];
        ASSERT(position_accessor.type == AccessorType::Vec3, "should be vec3");
        ASSERT(position_accessor.component_type == ComponentType::Float, "should be float");
        const GltfBufferView& position_buffer_view = buffer_views[position_accessor.buffer_view_index];

        const int32_t* normal_accessor_index = primitive.attributes.Find(String(scratch_allocator, "NORMAL"));
        ASSERT(normal_accessor_index, "should have a normal accessor");
        const GltfAccessor& normal_accessor = accessors[*normal_accessor_index];
        ASSERT(normal_accessor.type == AccessorType::Vec3, "should be vec3");
        ASSERT(normal_accessor.component_type == ComponentType::Float, "should be float");
        const GltfBufferView& normal_buffer_view = buffer_views[normal_accessor.buffer_view_index];

        const int32_t* texcoord0_accessor_index = primitive.attributes.Find(String(scratch_allocator, "TEXCOORD_0"));
        ASSERT(texcoord0_accessor_index, "should have a tex coord 0 accessor");
        const GltfAccessor& texcoord0_accessor = accessors[*texcoord0_accessor_index];
        ASSERT(texcoord0_accessor.type == AccessorType::Vec2, "should be vec2");
        ASSERT(texcoord0_accessor.component_type == ComponentType::Float, "should be float");
        const GltfBufferView& texcoord0_buffer_view = buffer_views[texcoord0_accessor.buffer_view_index];

        ASSERT(position_buffer_view.buffer_index == normal_buffer_view.buffer_index, "Should reference the same buffer");
        ASSERT(position_buffer_view.buffer_index == texcoord0_buffer_view.buffer_index, "Should reference the same buffer");
        const GltfBuffer& buffer = buffers[position_buffer_view.buffer_index];

        ASSERT(position_buffer_view.byte_offset < normal_buffer_view.byte_offset, "position should come first");
        ASSERT(normal_buffer_view.byte_offset < texcoord0_buffer_view.byte_offset, "normals should come second");

        const size_t buffer_start_offset = position_buffer_view.byte_offset;
        const size_t buffer_total_size = position_buffer_view.byte_length + normal_buffer_view.byte_length + texcoord0_buffer_view.byte_length;
        const float* buffer_start = (float*)(buffer.data + buffer_start_offset);

        ASSERT(position_accessor.count == normal_accessor.count, "Vertex attributes should have the same count of elements");
        ASSERT(position_accessor.count == texcoord0_accessor.count, "Vertex attributes should have the same count of elements");

        auto vbo = VertexBuffer::Create(alloc, buffer_start, buffer_total_size * sizeof(float));
        vbo->SetLayout(BufferLayout::NonInterleaved(alloc, {
            BufferLayoutDataType::Vec3, // position
            BufferLayoutDataType::Vec3, // normals
            BufferLayoutDataType::Vec2, // tex coords
        }, (size_t)position_accessor.count));

        submesh.vao = VertexArray::Create(mesh->allocator);
        submesh.vao->SetVertexBuffer(vbo);
        if (ibo) {
            ASSERT(ibo->GetNumIndices() == submesh.num_indices, "should be the same");
            submesh.vao->SetIndexBuffer(ibo);
        }

        mesh->sub_meshes.PushBack(std::move(submesh));
    }

    model.meshes.PushBack(std::move(mesh));

    scratch_allocator->Deallocate(data);
    return model;
}
