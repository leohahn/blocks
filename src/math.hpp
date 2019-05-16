#pragma once

#include "Math/Mat4.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec4.hpp"

namespace Math {

float
DegreesToRadians(float degrees)
{
    return degrees * (float)M_PI / 180.0f;
}

} // namespace Math
