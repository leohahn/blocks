#pragma once

#include "Defines.hpp"
#include "Collections/Array.hpp"
#include "Math/Vec3.hpp"
#include "Math/Quaternion.hpp"
#include "TriangleMesh.hpp"

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

