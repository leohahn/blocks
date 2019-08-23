#pragma once

#include "Han/Math/Float.hpp"
#include "Han/Math/Mat4.hpp"
#include "Han/Math/Vec3.hpp"
#include <assert.h>
#include <math.h>

struct Quaternion
{
    union
    {
        float val[4];
        struct
        {
            Vec3 v;
            float s;
        };
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
    };

    Quaternion()
        : Quaternion(0, 0, 0, 0)
    {}

    Quaternion(float x, float y, float z, float w)
        : x(x)
        , y(y)
        , z(z)
        , w(w)
    {}

    Quaternion(float s, const Vec3& v)
        : v(v)
        , s(s)
    {}

    static Quaternion Zero() { return Quaternion(); }

    static Quaternion Identity() { return Quaternion(0, 0, 0, 1); }

    static Quaternion Conjugate(const Quaternion& q) { return Quaternion(q.s, -q.v); }

    static Quaternion Rotate(const Quaternion& q, float angle, const Quaternion& axis)
    {
        const auto rotor = Quaternion::Rotation(angle, axis.v);
        // return rotor * q * Quaternion::Inverse(rotor);
        // NOTE: since we always use unit quaternions, we can use the conjugate here instead
        // of the inverse, since it is faster to compute
        return rotor * q * Quaternion::Conjugate(rotor);
    }

    static Quaternion Inverse(const Quaternion& q)
    {
        Quaternion inv = Quaternion::Conjugate(q) / Quaternion::SqrNorm(q);
        // assert(q * inv == Quaternion::Identity());
        return inv;
    }

    static Quaternion Rotation(float angle, const Vec3& axis)
    {
        const float half_angle = angle * 0.5f;
        const Vec3 sin_axis = axis * sinf(half_angle);
        return Quaternion(cosf(half_angle), sin_axis);
    }


    static float SqrNorm(const Quaternion& q)
    {
        float v = q.val[0]*q.val[0] + q.val[1]*q.val[1] + q.val[2]*q.val[2] + q.val[3]*q.val[3];
        return v;
    }

    static float Norm(const Quaternion& q)
    {
        return sqrtf(SqrNorm(q));
    }

    static Quaternion Slerp(const Quaternion& start_q, const Quaternion& end_q, float t);

    Mat4 ToMat4() const
    {
        auto mat = Mat4::Identity();
        const float qxx = x * x;
		const float qyy = y * y;
		const float qzz = z * z;
		const float qxz = x * z;
		const float qxy = x * y;
		const float qyz = y * z;
		const float qwx = w * x;
		const float qwy = w * y;
		const float qwz = w * z;

		mat.m00 = 1.0f - 2.0f * (qyy +  qzz);
		mat.m10 = 2.0f * (qxy + qwz);
		mat.m20 = 2.0f * (qxz - qwy);

		mat.m01 = 2.0f * (qxy - qwz);
		mat.m11 = 1.0f - 2.0f * (qxx +  qzz);
		mat.m21 = 2.0f * (qyz + qwx);

		mat.m02 = 2.0f * (qxz + qwy);
		mat.m12 = 2.0f * (qyz - qwx);
        mat.m22 = 1.0f - 2.0f * (qxx + qyy);

        return mat;
    }

    Quaternion operator+(const Quaternion& rhs) const
    {
        return Quaternion(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
    }

    Quaternion operator*(const Quaternion& rhs) const
    {
        // TODO: fix equation
        // pq = [(pSqV + qSpV + pV × qV) (pSqS − pV · qV)].
        return Quaternion((s * rhs.s) - Math::Dot(v, rhs.v),
                          rhs.s * v + s * rhs.v + Math::Cross(v, rhs.v));
    }

    Quaternion operator/(float k) const
    {
        return Quaternion(x / k, y / k, z / k, w / k);
    }

    static void Print(const Quaternion& q);
};

inline Quaternion operator*(const Quaternion& q, float k)
{
    return Quaternion(q.s * k, q.v * k);
}

inline Quaternion operator*(float k, const Quaternion& q)
{
    return Quaternion(q.s * k, q.v * k);
}

inline bool
operator==(const Quaternion& a, const Quaternion& b)
{
    return Math::IsAlmostEqual(a.val[0], b.val[0]) && Math::IsAlmostEqual(a.val[1], b.val[1]) &&
           Math::IsAlmostEqual(a.val[2], b.val[2]) && Math::IsAlmostEqual(a.val[3], b.val[3]);
}

inline bool
operator!=(const Quaternion& a, const Quaternion& b)
{
    return !Math::IsAlmostEqual(a.val[0], b.val[0]) || !Math::IsAlmostEqual(a.val[1], b.val[1]) ||
           !Math::IsAlmostEqual(a.val[2], b.val[2]) || !Math::IsAlmostEqual(a.val[3], b.val[3]);
}

namespace Math {

inline Quaternion
Normalize(const Quaternion& q)
{
    return Quaternion(q.x / Quaternion::Norm(q),
                      q.y / Quaternion::Norm(q),
                      q.z / Quaternion::Norm(q),
                      q.w / Quaternion::Norm(q));
}

inline float
Dot(const Quaternion& a, const Quaternion& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

} // namespace Math
