#pragma once

#include "Allocator.hpp"
#include "Collections/Array.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec4.hpp"
#include "TriangleMesh.hpp"

void RenderMesh(const TriangleMesh& mesh,
                Vec3 position,
                Quaternion orientation,
                float mesh_scale,
                Vec4* scale_color = nullptr,
                Vec4* override_color = nullptr);
