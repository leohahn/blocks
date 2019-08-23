#pragma once

#include "Han/Allocator.hpp"
#include "Han/Collections/Array.hpp"
#include "Han/Math/Quaternion.hpp"
#include "Han/Math/Vec3.hpp"
#include "Han/Math/Vec4.hpp"
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
