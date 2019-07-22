#include "Importers/GLTF2.hpp"
#include "FileSystem.hpp"
#include "Json.hpp"
#include "Logger.hpp"
#include "ResourceManager.hpp"

Model
ImportGltf2Model(Allocator* allocator, Allocator* scratch_allocator, const Path& path, ResourceManager* resource_manager)
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
    
    // 1. first, load all of the textures to memory
    const RobinHashMap<String, Json::Val>* root = doc.root_val.AsObject();
    assert(root);
    
    const Json::Val* images = root->Find(String(scratch_allocator, "images"));
    if (images == nullptr || !images->IsArray()) {
        LOG_ERROR("Expecting an images array");
        assert(false);
    }
    
    for (size_t i = 0; i < images->AsArray()->len; ++i) {
        const RobinHashMap<String, Json::Val>* obj = (*images->AsArray())[i].AsObject();
        if (!obj) {
            LOG_ERROR("Image element in array is not a JSON object");
            assert(false);
        }
        
        const Json::Val* mime_type = obj->Find(String(scratch_allocator, "mimeType"));
        if (!mime_type || !mime_type->IsString()) {
            LOG_ERROR("Mime type is invalid");
            assert(false);
        }
        
        if (*mime_type->AsString() != "image/png") {
            LOG_ERROR("Unsupported image format: %s", mime_type->AsString()->data);
            assert(false);
        }

        const Json::Val* name = obj->Find(String(scratch_allocator, "name"));
        if (!name || !name->IsString()) {
            LOG_ERROR("Name is invalid");
            assert(false);
        }
        
        const Json::Val* uri = obj->Find(String(scratch_allocator, "uri"));
        if (!uri || !uri->IsString()) {
            LOG_ERROR("uri type is invalid");
            assert(false);
        }
        
        LOG_DEBUG("Loading GLTF texture %s at %s", name->AsString()->data, uri->AsString()->data);

        Sid uri_sid = SID(uri->AsString()->data);
        //resource_manager->LoadTexture(uri_sid);
    }

    const Json::Val* textures = root->Find(String(scratch_allocator, "textures"));
    if (textures == nullptr || !textures->IsArray()) {
        LOG_ERROR("Expecting a textures array");
        assert(false);
    }
    

    scratch_allocator->Deallocate(data);
    return Model(allocator);
}
