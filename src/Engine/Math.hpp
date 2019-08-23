#pragma once

#include "Han/Math/Mat4.hpp"
#include "Han/Math/Quaternion.hpp"
#include "Han/Math/Vec2.hpp"
#include "Han/Math/Vec3.hpp"
#include "Han/Math/Vec4.hpp"

namespace Math {

float
DegreesToRadians(float degrees)
{
    return degrees * (float)M_PI / 180.0f;
}

} // namespace Math
