#ifndef BLOCKS_MATH_VEC3_HPP
#define BLOCKS_MATH_VEC3_HPP

#include <math.h>

struct Vec3
{
    union 
    {
        struct
        {
            float x;
            float y;
            float z;
        };
        struct
        {
            float i;
            float j;
            float k;
        };
    };

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

    void operator-=(const Vec3& vec)
    {
        x -= vec.x; y -= vec.y; z -= vec.z;
    }

    void operator+=(const Vec3& vec)
    {
        x += vec.x; y += vec.y; z += vec.z;
    }

    Vec3 operator-() const
    {
        return Vec3::New(-x, -y, -z);
    }
};

inline Vec3
operator+(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3::New(
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z
    );
}

inline Vec3
operator*(const Vec3& v, float k)
{
    return Vec3::New(
        v.x * k,
        v.y * k,
        v.z * k
    );
}

inline Vec3
operator*(float k, const Vec3& v)
{
    return Vec3::New(
        v.x * k,
        v.y * k,
        v.z * k
    );
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
