#pragma once

#include "Allocator.hpp"
#include "Collections/Array.hpp"
#include "Collections/StringView.hpp"
#include "TriangleMesh.hpp"

struct TriangleMeshCatalog
{
    Allocator* allocator = nullptr;
    Allocator* scratch_allocator = nullptr;
    Array<TriangleMesh> meshes;

public:
    TriangleMeshCatalog(Allocator* allocator, Allocator* scratch_allocator)
        : allocator(allocator)
        , scratch_allocator(scratch_allocator)
        , meshes(allocator)
    {}

    ~TriangleMeshCatalog() { assert(!allocator); }

    void Destroy()
    {
        for (size_t i = 0; i < meshes.len; ++i) {
            meshes[i].Destroy();
        }
        meshes.Destroy();
        allocator = nullptr;
        scratch_allocator = nullptr;
    }

    void LoadMeshFromFile(const StringView& mesh_file);
};
