#include "Importers/GLTF2.hpp"
#include "FileSystem.hpp"
#include "Json.hpp"
#include "Logger.hpp"
#include "ResourceManager.hpp"

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
GetRotation(const Json::Val* rotation, Quaternion* out)
{
    assert(out);
    const Array<Json::Val>* rotation_array = rotation->AsArray();
    if (!rotation_array) {
        return false;
    }
    
    if (rotation_array->len != 4) {
        return false;
    }
    
    const double* x = (*rotation_array)[0].AsDouble();
    const double* y = (*rotation_array)[1].AsDouble();
    const double* z = (*rotation_array)[2].AsDouble();
    const double* w = (*rotation_array)[3].AsDouble();
    
    if (!x || !y || !z || !w) {
        return false;
    }
    
    *out = Quaternion(*x, *y, *z, *w);
    return true;
}
        
static bool
GetVec3(const Json::Val* vec, Vec3* out)
{
    assert(out);
    const Array<Json::Val>* translation_array = vec->AsArray();
    if (!translation_array) {
        return false;
    }
    
    if (translation_array->len != 3) {
        return false;
    }
    
    const double* x = (*translation_array)[0].AsDouble();
    const double* y = (*translation_array)[1].AsDouble();
    const double* z = (*translation_array)[2].AsDouble();
    
    if (!x || !y || !z) {
        return false;
    }

    *out = Vec3(*x, *y, *z);
    return true;
}

bool
GetNodes(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfNode>* out_nodes)
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

        if (!GetVec3(translation_val, &out_node.translation)) {
            LOG_ERROR("Failed to parse translation vector in node");
            return false;
        }

        if (!GetRotation(rotation_val, &out_node.rotation)) {
            LOG_ERROR("Failed to parse rotation vector in node");
            return false;
        }

        out_nodes->PushBack(std::move(out_node));
    }
    
    return true;
}

bool
GetMeshes(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfMesh>* out_meshes)
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
            primitive.indices = (int32_t)raw_indices->AsInt64();
            primitive.material = (int32_t)raw_indices->AsInt64();
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
GetBuffers(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfBuffer>* out_buffers)
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
        out_buf.uri = String(alloc, byte_length_val->AsString()->View());
        out_buffers->PushBack(out_buf);
    }
    
    return true;
}

static bool
GetAccessors(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfAccessor>* out_accessors)
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
        if (!max_val || !max_val->IsArray()) {
            LOG_ERROR("Was expecting a max property");
            return false;
        }

        const Json::Val* min_val = accessor->Find(String(alloc, "min"));
        if (!min_val || !min_val->IsArray()) {
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

        if (!GetVec3(max_val, &out_accessor.max)) {
            LOG_ERROR("Was expecting a max vector");
            return false;
        }

        if (!GetVec3(min_val, &out_accessor.min)) {
            LOG_ERROR("Was expecting a min vector");
            return false;
        }

        out_accessors->PushBack(std::move(out_accessor));
    }
    
    return true;
}


static bool
GetMaterial(Allocator* alloc, const RobinHashMap<String, Json::Val>* raw_material, GltfMaterial* out_material)
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

    const Json::Val* metallic_roughness_texture_index = base_color_texture->AsObject()->Find(String(alloc, "index"));
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
GetMaterials(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfMaterial>* out_materials)
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
        if (!GetMaterial(alloc, raw_material, &material)) {
            LOG_ERROR("Was expecting a material object");
            return false;
        }

        out_materials->PushBack(std::move(material));
    }
    
    return true;
}

static bool
GetImages(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfImage>* out_images)
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
GetBufferViews(Allocator* alloc, const RobinHashMap<String, Json::Val>* gltf_file, Array<GltfBufferView>* out_buffer_views)
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
    if (!GetBuffers(alloc, root, &buffers)) {
        LOG_ERROR("Was expecting a buffers array");
        assert(false);
    }

    Array<GltfMesh> meshes;
    if (!GetMeshes(alloc, root, &meshes)) {
        LOG_ERROR("Was expecting a meshes array");
        assert(false);
    }

    Array<GltfNode> nodes;
    if (!GetNodes(alloc, root, &nodes)) {
        LOG_ERROR("Was expecting a nodes array");
        assert(false);
    }

    Array<GltfMaterial> materials;
    if (!GetMaterials(alloc, root, &materials)) {
        LOG_ERROR("Was expecting a materials array");
        assert(false);
    }

    Array<GltfImage> images;
    if (!GetImages(alloc, root, &images)) {
        LOG_ERROR("Was expecting an images array");
        assert(false);
    }

    Array<GltfAccessor> accessors;
    if (!GetAccessors(alloc, root, &accessors)) {
        LOG_ERROR("Was expecting an accessors array");
        assert(false);
    }

    Array<GltfBufferView> buffer_views;
    if (!GetBufferViews(alloc, root, &buffer_views)) {
        LOG_ERROR("Was expecting a bufferViews array");
        assert(false);
    }

    //===============================================================
    // TODO: actually load file into memory
    //===============================================================

    scratch_allocator->Deallocate(data);
    return Model(alloc);
}
