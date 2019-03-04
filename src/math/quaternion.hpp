#ifndef BLOCKS_MATH_QUATERNION_HPP
#define BLOCKS_MATH_QUATERNION_HPP

#include <math.h>
#include <assert.h>
#include "math/vec3.hpp"
#include "math/mat4.hpp"
#include "math/float.hpp"

struct Quaternion
{
    union
    {
        float val[4];
        struct
        {
            float s;
            Vec3 v;
        };
    };

    static Quaternion New(float s, float i, float j, float k)
    {
        Quaternion q;
        q.val[0] = s;
        q.val[1] = i;
        q.val[2] = j;
        q.val[3] = k;
        return q;
    }

    static Quaternion New(float s, const Vec3& v)
    {
        Quaternion q;
        q.s = s;
        q.v = v;
        return q;
    }

    static Quaternion Zero()
    {
        Quaternion q = {};
        return q;
    }

    static Quaternion Identity()
    {
        return Quaternion::New(1, 0, 0, 0);
    }

    static Quaternion Conjugate(const Quaternion& q)
    {
        return Quaternion::New(q.s, -q.v);
    }

    static Quaternion Inverse(const Quaternion& q);

    static Quaternion Rotation(float angle, const Vec3& axis);

    static float Norm(const Quaternion& q);

    static float SqrNorm(const Quaternion& q);

    static Quaternion Slerp(const Quaternion& start_q, const Quaternion& end_q, float t);

    Mat4
    ToMat4() const
    {
        return Mat4::New(s,   -v.i, -v.j, -v.k,
                         v.i,    s, -v.k,  v.j,
                         v.j,  v.k,    s, -v.i,
                         v.k, -v.j,  v.i,    s);
    }

    Quaternion
    operator+(const Quaternion& rhs) const
    {
        return Quaternion::New(s+rhs.s, v.i+rhs.v.i, v.j+rhs.v.j, v.k+rhs.v.k);
    }

    Quaternion
    operator/(float k) const
    {
        return Quaternion::New(val[0]/k, val[1]/k, val[2]/k, val[3]/k);
    }
};

inline Quaternion
operator*(const Quaternion& q, float k)
{
    return Quaternion::New(q.s*k, q.v*k);
}

inline Quaternion
operator*(float k, const Quaternion& q)
{
    return Quaternion::New(q.s*k, q.v*k);
}

inline Quaternion
operator*(const Quaternion& lhs, const Quaternion& rhs)
{
    return Quaternion::New((lhs.s*rhs.s) - Math::Dot(lhs.v, rhs.v),
                   rhs.s*lhs.v + lhs.s*rhs.v + Math::Cross(lhs.v, rhs.v));
}

inline bool
operator==(const Quaternion& a, const Quaternion& b)
{
    return Math::IsAlmostEqual(a.val[0], b.val[0]) &&
           Math::IsAlmostEqual(a.val[1], b.val[1]) &&
           Math::IsAlmostEqual(a.val[2], b.val[2]) &&
           Math::IsAlmostEqual(a.val[3], b.val[3]);
}

namespace Math
{

inline Quaternion
Normalize(const Quaternion& q)
{
    return Quaternion::New(
        q.s/Quaternion::Norm(q),
        q.v.i/Quaternion::Norm(q),
        q.v.j/Quaternion::Norm(q),
        q.v.k/Quaternion::Norm(q)
    );
}

inline Quaternion
rotate(const Quaternion& q, float angle, const Quaternion& axis)
{
    const Quaternion rotor = Quaternion::Rotation(angle, axis.v);
    return rotor * q * Quaternion::Inverse(rotor);
}

inline float
Dot(const Quaternion& a, const Quaternion& b)
{
	return a.val[0]*b.val[0] + a.val[1]*b.val[1] + a.val[2]*b.val[2] + a.val[3]*b.val[3];
}

}

#endif // BLOCKS_MATH_QUATERNION_HPP