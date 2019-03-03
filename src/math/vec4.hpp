#ifndef BLOCKS_MATH_VEC4_HPP
#define BLOCKS_MATH_VEC4_HPP

#include "math/vec3.hpp"

struct Vec4
{
    float x;
    float y;
    float z;
    float w;

    static Vec4 New(float x, float y, float z, float w)
    {
        Vec4 v;
        v.x = x;
        v.y = y;
        v.z = z;
        v.w = w;
        return v;
    }

    static Vec4 New(float val)
    {
        Vec4 v;
        v.x = val;
        v.y = val;
        v.z = val;
        v.w = val;
        return v;
    }

    static Vec4 New(Vec3 v, float w)
    {
        Vec4 ret;
        ret.x = v.x;
        ret.y = v.y;
        ret.z = v.z;
        ret.w = w;
        return ret;
    }
};

inline Vec4
operator+(Vec4 lhs, Vec4 rhs)
{
    Vec4 res;
    res.x = lhs.x + rhs.x;
    res.y = lhs.y + rhs.y;
    res.z = lhs.z + rhs.z;
    res.w = lhs.w + rhs.w;
    return res;
}

inline Vec4
operator-(Vec4 lhs, Vec4 rhs)
{
    Vec4 res;
    res.x = lhs.x - rhs.x;
    res.y = lhs.y - rhs.y;
    res.z = lhs.z - rhs.z;
    res.w = lhs.w - rhs.w;
    return res;
}

namespace Math
{
    inline float
    DotProduct(Vec4 v1, Vec4 v2)
    {
        return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w;
    }
}

#endif // BLOCKS_MATH_VEC4_HPP
