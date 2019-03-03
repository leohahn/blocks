#ifndef BLOCKS_MATH_VEC3_HPP
#define BLOCKS_MATH_VEC3_HPP

#include <math.h>

struct Vec3
{
    float x;
    float y;
    float z;

    static Vec3 New(float x, float y, float z)
    {
        Vec3 v;
        v.x = x;
        v.y = y;
        v.z = z;
        return v;
    }

    static Vec3 New(float val)
    {
        Vec3 v;
        v.x = val;
        v.y = val;
        v.z = val;
        return v;
    }

    static Vec3 Zero()
    {
        return Vec3::New(0.0f);
    }
};

inline Vec3
operator+(Vec3 lhs, Vec3 rhs)
{
    Vec3 res;
    res.x = lhs.x + rhs.x;
    res.y = lhs.y + rhs.y;
    res.z = lhs.z + rhs.z;
    return res;
}

inline Vec3
operator-(Vec3 lhs, Vec3 rhs)
{
    Vec3 res;
    res.x = lhs.x - rhs.x;
    res.y = lhs.y - rhs.y;
    res.z = lhs.z - rhs.z;
    return res;
}

namespace Math
{
    inline float
    Dot(const Vec3& v1, const Vec3& v2)
    {
        return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
    }

    inline Vec3
    Normalize(const Vec3& vec)
    {
        float len = sqrtf(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
        return Vec3::New(vec.x/len, vec.y/len, vec.z/len);
    }

    inline Vec3
    Cross(const Vec3& vec_a, const Vec3& vec_b)
    {
        return Vec3::New(
            vec_a.y * vec_b.z - vec_a.z * vec_b.y,
            vec_a.z * vec_b.x - vec_a.x * vec_b.z,
            vec_a.x * vec_b.y - vec_a.y * vec_b.x
        );
    }
}

#endif // BLOCKS_MATH_VEC3_HPP
