#pragma once

#include "Math/Vec3.hpp"

struct Vec4
{
    union
    {
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };

        struct
        {
            Vec3 xyz;
            float _unused;
        };
    };


    Vec4() = default;

    Vec4(float x, float y, float z, float w)
        : x(x)
        , y(y)
        , z(z)
        , w(w)
    {}

    Vec4(float val)
        : Vec4(val, val, val, val)
    {}

    Vec4(Vec3 v, float w)
        : Vec4(v.x, v.y, v.z, w)
    {}
};

inline Vec4
operator+(Vec4 lhs, Vec4 rhs)
{
    return Vec4(
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z,
        lhs.w + rhs.w
    );
}

inline Vec4
operator-(Vec4 lhs, Vec4 rhs)
{
    return Vec4(
        lhs.x - rhs.x,
        lhs.y - rhs.y,
        lhs.z - rhs.z,
        lhs.w - rhs.w
    );
}

namespace Math
{
    inline float
    DotProduct(Vec4 v1, Vec4 v2)
    {
        return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w;
    }
}
