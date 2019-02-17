#ifndef BLOCKS_MATH_VEC3_HPP
#define BLOCKS_MATH_VEC3_HPP

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

#endif // BLOCKS_MATH_VEC3_HPP
