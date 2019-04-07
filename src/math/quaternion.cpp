#include "Math/Quaternion.hpp"
#include <stdio.h>

void 
Quaternion::Print(const Quaternion& q)
{
    printf("s = %.2f, vx = %.2f, vy = %.2f, vz = %.2f\n",
           q.s, q.v.x, q.v.y, q.v.z);
}

Quaternion
Quaternion::Inverse(const Quaternion& q)
{
    Quaternion inv = Quaternion::Conjugate(q) / Quaternion::SqrNorm(q);
    assert(q*inv == Quaternion::Identity());
    return inv;
}

Quaternion
Quaternion::Rotation(float angle, const Vec3& axis)
{
    Vec3 sin_axis = axis * sinf(angle/2.0f);
    return Quaternion(cosf(angle/2.0f), sin_axis);
}

float
Quaternion::Norm(const Quaternion& q)
{
    float v = q.val[0]*q.val[0] + q.val[1]*q.val[1] + q.val[2]*q.val[2] + q.val[3]*q.val[3];
    return sqrtf(v);
}

float
Quaternion::SqrNorm(const Quaternion& q)
{
    float v = q.val[0]*q.val[0] + q.val[1]*q.val[1] + q.val[2]*q.val[2] + q.val[3]*q.val[3];
    return v;
}

Quaternion
Quaternion::Slerp(const Quaternion& start_q, const Quaternion& end_q, float t)
{
    assert(t >= 0);
    assert(t <= 1);

    const float EPSILON = 0.0001f; // FIXME: Find a more specific epsilon value here.
    float start_dot_end = Math::Dot(start_q, end_q);

    if (start_dot_end < 1-EPSILON)
    {
        float angle = acos(start_dot_end);
        assert(angle != 0.0f);
        return (sinf((1.0f - t) * angle) * start_q + sin(t * angle) * end_q) /
            sinf(angle);
    }
    else
    {
        return start_q;
    }
}
