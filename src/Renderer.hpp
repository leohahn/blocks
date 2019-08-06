#pragma once

#include "Allocator.hpp"
#include "Collections/Array.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec4.hpp"
#include "TriangleMesh.hpp"
#include "Shader.hpp"
#include "Model.hpp"

void RenderMesh(const TriangleMesh& mesh,
                const Shader& shader,
                Vec3 position,
                Quaternion orientation,
                float mesh_scale,
                Vec4* scale_color = nullptr);

void RenderModel(const Model& model,
                 const Shader& shader,
                 Vec3 position,
                 Quaternion orientation,
                 float mesh_scale,
                 Vec4* scale_color = nullptr);
