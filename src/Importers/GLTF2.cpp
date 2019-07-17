#include "Importers/GLTF2.hpp"
#include "FileSystem.hpp"
#include "Json.hpp"
#include "Logger.hpp"

Model
ImportGltf2Model(Allocator* allocator, Allocator* scratch_allocator, const Path& path)
{
    size_t size;
    uint8_t* data = FileSystem::LoadFileToMemory(scratch_allocator, path, &size);
    assert(data);

    Json::Document doc(scratch_allocator);
    doc.Parse(data, size);
    if (doc.HasParseErrors()) {
        LOG_ERROR("GLTF2 JSON HAS PARSE ERRORS: %s", doc.GetErrorStr());
        assert(false);
    }

    assert(doc.root_val.type == Json::Type::Object);

    // -------------------------------------------------------------------
    //
    // TODO: implement importing of gltf
    //
    // -------------------------------------------------------------------

    scratch_allocator->Deallocate(data);

    return Model(allocator);
}
