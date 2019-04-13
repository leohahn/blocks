#include "Math/Quaternion.hpp"
#include <stdio.h>

void 
Quaternion::Print(const Quaternion& q)
{
    printf("s = %.2f, vx = %.2f, vy = %.2f, vz = %.2f\n",
           q.s, q.v.x, q.v.y, q.v.z);
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
