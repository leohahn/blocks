#pragma once

#include "Han/Core.hpp"
#include "Han/Collections/Array.hpp"
#include "Han/Math/Vec3.hpp"
#include "Han/Math/Quaternion.hpp"
#include "TriangleMesh.hpp"

struct Model
{
    // TODO: transform a model into two different classes:
    // a model and a model instance. A model instance will be a light weight model
    // with orientation, scale, etc.
    Sid name;
    Array<TriangleMesh*> meshes;
    Vec3 translation;
    Quaternion rotation;
    float scale;

public:
    Model(Allocator* allocator)
        : meshes(allocator)
    {}
    
    Model(Model&& model) = default;

    DISABLE_OBJECT_COPY(Model);
};

