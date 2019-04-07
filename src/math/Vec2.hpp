#pragma once

#include "Math/Float.hpp"
#include <math.h>
#include <stdio.h>

struct Vec2
{
    union
    {
        struct
        {
            float x;
            float y;
        };
        struct
        {
            float i;
            float j;
        };
    };

    Vec2() = default;

    Vec2(float x, float y)
        : x(x)
        , y(y)
    {}

    Vec2(float val)
        : x(val)
        , y(val)
    {}

    static Vec2 Zero() { return Vec2(0.0f); }

    void operator-=(const Vec2& vec)
    {
        x -= vec.x;
        y -= vec.y;
    }

    void operator+=(const Vec2& vec)
    {
        x += vec.x;
        y += vec.y;
    }

    Vec2 operator-() const { return Vec2(-x, -y); }

    void Negate()
    {
        x = -x;
        y = -y;
    }
};

inline Vec2
operator+(const Vec2& lhs, const Vec2& rhs)
{
    return Vec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

inline Vec2 operator*(const Vec2& v, float k)
{
    return Vec2(v.x * k, v.y * k);
}

inline bool
operator==(const Vec2& lhs, const Vec2& rhs)
{
    return Math::IsAlmostEqual(lhs.x, rhs.x) && Math::IsAlmostEqual(lhs.y, rhs.y);
}

inline bool
operator!=(const Vec2& lhs, const Vec2& rhs)
{
    return !Math::IsAlmostEqual(lhs.x, rhs.x) || !Math::IsAlmostEqual(lhs.y, rhs.y);
}

inline Vec2 operator*(float k, const Vec2& v)
{
    return Vec2(v.x * k, v.y * k);
}

inline Vec2
operator-(Vec2 lhs, Vec2 rhs)
{
    return Vec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

namespace Math {

inline float
Dot(const Vec2& v1, const Vec2& v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

inline Vec2
Normalize(const Vec2& vec)
{
    float len = sqrtf(vec.x * vec.x + vec.y * vec.y);
    return Vec2(vec.x / len, vec.y / len);
}

} // namespace Math
