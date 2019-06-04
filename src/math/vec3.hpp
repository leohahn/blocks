#pragma once

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include "Math/Float.hpp"

struct Vec3i
{
    union 
    {
        struct
        {
            int32_t x;
            int32_t y;
            int32_t z;
        };
        struct
        {
            int32_t i;
            int32_t j;
            int32_t k;
        };
    };

    Vec3i() = default;

    Vec3i(int32_t x, int32_t y, int32_t z)
        : x(x), y(y), z(z)
    {}

    void operator-=(const Vec3i& vec)
    {
        x -= vec.x; y -= vec.y; z -= vec.z;
    }

    void operator+=(const Vec3i& vec)
    {
        x += vec.x; y += vec.y; z += vec.z;
    }

    Vec3i operator-() const
    {
        return Vec3i(-x, -y, -z);
    }

    void Negate() { x = -x; y = -y; z = -z; }
};

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

    Vec3() = default;

    Vec3(float x, float y, float z)
        : x(x), y(y), z(z)
    {}

    Vec3(float val)
        : x(val)
        , y(val)
        , z(val)
    {} 

    static Vec3 Zero()
    {
        return Vec3(0.0f);
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
        return Vec3(-x, -y, -z);
    }

    void Negate() { x = -x; y = -y; z = -z; }
};

inline Vec3
operator+(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3(
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z
    );
}

inline Vec3
operator*(const Vec3& v, float k)
{
    return Vec3(
        v.x * k,
        v.y * k,
        v.z * k
    );
}

inline bool
operator==(const Vec3& lhs, const Vec3& rhs)
{
    return Math::IsAlmostEqual(lhs.x, rhs.x) &&
           Math::IsAlmostEqual(lhs.y, rhs.y) &&
           Math::IsAlmostEqual(lhs.z, rhs.z);
}

inline bool
operator!=(const Vec3& lhs, const Vec3& rhs)
{
    return !Math::IsAlmostEqual(lhs.x, rhs.x) ||
           !Math::IsAlmostEqual(lhs.y, rhs.y) ||
           !Math::IsAlmostEqual(lhs.z, rhs.z);
}

inline Vec3
operator*(float k, const Vec3& v)
{
    return Vec3(
        v.x * k,
        v.y * k,
        v.z * k
    );
}

inline Vec3
operator-(Vec3 lhs, Vec3 rhs)
{
    return Vec3(
        lhs.x - rhs.x,
        lhs.y - rhs.y,
        lhs.z - rhs.z
    );
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
        return Vec3(vec.x/len, vec.y/len, vec.z/len);
    }

    inline Vec3
    Cross(const Vec3& vec_a, const Vec3& vec_b)
    {
        return Vec3(
            vec_a.y * vec_b.z - vec_a.z * vec_b.y,
            vec_a.z * vec_b.x - vec_a.x * vec_b.z,
            vec_a.x * vec_b.y - vec_a.y * vec_b.x
        );
    }
}
