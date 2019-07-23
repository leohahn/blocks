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
    int32_t index;
    int32_t tex_coord;
};

struct GltfMaterial
{
    String name;
    bool doubleSided;
    TextureRef base_color;
    TextureRef metallic_roughness;
};

struct GltfTexture
{
    int32_t source;
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
    int32_t indices;
    int32_t material;
};

struct GltfMesh
{
    String name;
    Array<GltfPrimitive> primitives;
};

struct GltfBuffer
{
    int64_t byte_length;
    String uri;
};

struct GltfBufferView
{
    int64_t buffer_index;
    int64_t byte_length;
    int64_t byte_offset;
};

struct GltfAccessor
{
    String type;
    int64_t buffer_view_index;
    int64_t component_type;
    int64_t count;
    Vec3 max;
    Vec3 min;
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
GetTranslation(const Json::Val* translation, Vec3* out)
{
    assert(out);
    const Array<Json::Val>* translation_array = translation->AsArray();
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
            const RobinHashMap<<#typename Key#>, <#typename Value#>>

            GltfPrimitive primitive;
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
GetMaterial(Allocator* alloc, const Json::Val* materials,
            const Json::Val* mesh_material, ResourceManager* resource_manager, Material* out)
{
    assert(out);

    if (!materials || !mesh_material) {
        return false;
    }
    
    const Array<Json::Val>* materials_array = materials->AsArray();
    const int64_t* mesh_index = mesh_material->AsInt64();
    
    if (!mesh_index || !materials_array) {
        return false;
    }

    if (materials_array->len < *mesh_index + 1) {
        return false;
    }
    
    const RobinHashMap<String, Json::Val>* selected_material = (*materials_array)[*mesh_index].AsObject();
    if (!selected_material) {
        return false;
    }
    
    const Json::Val* material_name = selected_material->Find(String(alloc, "name"));
    if (!material_name || !material_name->AsString()) {
        LOG_ERROR("Was expecting material name");
        assert(false);
    }
    
    const Json::Val* pbr_params = selected_material->Find(String(alloc, "pbrMetallicRoughness"));
    if (!pbr_params || !pbr_params->AsObject()) {
        LOG_ERROR("Was expecting pbr metallic roughness");
        assert(false);
    }
    
    const Json::Val* base_color_texture = pbr_params->AsObject()->Find(String(alloc, "baseColorTexture"));
    if (!base_color_texture || !base_color_texture->IsObject()) {
        LOG_ERROR("Expecting baseColorTexture");
        assert(false);
    }
    
    const Json::Val* base_color_texture_index = base_color_texture->AsObject()->Find(String(alloc, "index"));
    const Json::Val* base_color_texture_tex_coord = base_color_texture->AsObject()->Find(String(alloc, "texCoord"));

    Material out_mat;
    out_mat.name = SID(material_name->AsString()->data);
    
    // Load the diffuse map

    *out = std::move(out_mat);
    return true;
}

Model
ImportGltf2Model(Allocator* allocator, Allocator* scratch_allocator, const Path& path, ResourceManager* resource_manager, int model_index)
{
    size_t size;
    uint8_t* data = FileSystem::LoadFileToMemory(scratch_allocator, path, &size);
    assert(data);
    
    Model model(allocator);

    Json::Document doc(scratch_allocator);
    doc.Parse(data, size);
    if (doc.HasParseErrors() || !doc.root_val.IsObject()) {
        LOG_ERROR("GLTF2 file is corrupt: %s", doc.GetErrorStr());
        assert(false);
    }

    assert(doc.root_val.type == Json::Type::Object);
    
    const RobinHashMap<String, Json::Val>* root = doc.root_val.AsObject();
    assert(root);
    
    // 1. First, we will load a specific node from the GLTF model, and load the dependencies as
    // necessary.
    const Json::Val* nodes = root->Find(String(scratch_allocator, "nodes"));
    if (!nodes || !nodes->IsArray() || nodes->AsArray()->len < model_index + 1) {
        LOG_ERROR("Expecting nodes array");
        assert(false);
    }
    
    const Json::Val* materials = root->Find(String(scratch_allocator, "materials"));
    
    const RobinHashMap<String, Json::Val>* selected_node = (*nodes->AsArray())[model_index].AsObject();
    assert(selected_node);
    
    const Json::Val* model_name = selected_node->Find(String(scratch_allocator, "name"));
    if (model_name && model_name->IsString()) {
        model.name = SID(model_name->AsString()->data);
    }
    
    const Json::Val* model_mesh = selected_node->Find(String(scratch_allocator, "mesh"));
    assert(model_mesh && model_mesh->IsInteger());
    
    int64_t mesh_index = *model_mesh->AsInt64();

    const Json::Val* model_rotation = selected_node->Find(String(scratch_allocator, "rotation"));
    const Json::Val* model_translation = selected_node->Find(String(scratch_allocator, "translation"));
    const Json::Val* model_children = selected_node->Find(String(scratch_allocator, "children"));
    if (model_children) {
        LOG_ERROR("Model children not supported yet");
        assert(false);
    }
    
    assert(model_rotation && model_rotation->IsArray());
    assert(model_translation && model_translation->IsArray());

    if (!GetTranslation(model_translation, &model.translation)) {
        LOG_ERROR("Failed to get model translation");
        assert(false);
    }
    
    if (!GetRotation(model_rotation, &model.rotation)) {
        LOG_ERROR("Failed to get model orientation");
        assert(false);
    }
    
    // 2. Having parsed everything from the node object, start parsing the mesh.
    const Json::Val* meshes = root->Find(String(scratch_allocator, "meshes"));
    if (meshes == nullptr || !meshes->IsArray() || meshes->AsArray()->len < mesh_index + 1) {
        LOG_ERROR("Expecting a meshes array");
        assert(false);
    }
    
    const RobinHashMap<String, Json::Val>* selected = (*meshes->AsArray())[mesh_index].AsObject();
    assert(selected);

    const Json::Val* mesh_name = selected->Find(String(scratch_allocator, "name"));
    const Json::Val* mesh_primitives = selected->Find(String(scratch_allocator, "primitives"));
    const Json::Val* mesh_material = selected->Find(String(scratch_allocator, "material"));
    
    assert(mesh_material);
    assert(mesh_name);
    assert(mesh_primitives);
    
    Material material;
    if (!GetMaterial(scratch_allocator, materials, mesh_material, &material)) {
        LOG_ERROR("Failed parsing material from GLTF file");
        assert(false);
    }

    TriangleMesh mesh(allocator);

//    for (size_t i = 0; i < images->AsArray()->len; ++i) {
//        const RobinHashMap<String, Json::Val>* obj = (*images->AsArray())[i].AsObject();
//        if (!obj) {
//            LOG_ERROR("Image element in array is not a JSON object");
//            assert(false);
//        }
//
//        const Json::Val* mime_type = obj->Find(String(scratch_allocator, "mimeType"));
//        if (!mime_type || !mime_type->IsString()) {
//            LOG_ERROR("Mime type is invalid");
//            assert(false);
//        }
//
//        if (*mime_type->AsString() != "image/png") {
//            LOG_ERROR("Unsupported image format: %s", mime_type->AsString()->data);
//            assert(false);
//        }
//
//        const Json::Val* name = obj->Find(String(scratch_allocator, "name"));
//        if (!name || !name->IsString()) {
//            LOG_ERROR("Name is invalid");
//            assert(false);
//        }
//
//        const Json::Val* uri = obj->Find(String(scratch_allocator, "uri"));
//        if (!uri || !uri->IsString()) {
//            LOG_ERROR("uri type is invalid");
//            assert(false);
//        }
//
//        LOG_DEBUG("Loading GLTF texture %s at %s", name->AsString()->data, uri->AsString()->data);
//        Texture* texture = resource_manager->LoadTexture(SID(uri->AsString()->data));
//        (void)texture;
//    }

//    const Json::Val* textures = root->Find(String(scratch_allocator, "textures"));
//    if (textures == nullptr || !textures->IsArray()) {
//        LOG_ERROR("Expecting a textures array");
//        assert(false);
//    }
    

    scratch_allocator->Deallocate(data);
    return Model(allocator);
}
