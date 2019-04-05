#include "Math/Mat4.hpp"

Vec4
operator*(const Mat4& lhs, const Vec4& rhs)
{
    Vec4 ret;
    ret.x = lhs.m00 * rhs.x + lhs.m01 * rhs.y + lhs.m02 * rhs.z + lhs.m03 * rhs.w;
    ret.y = lhs.m10 * rhs.x + lhs.m11 * rhs.y + lhs.m12 * rhs.z + lhs.m13 * rhs.w;
    ret.z = lhs.m20 * rhs.x + lhs.m21 * rhs.y + lhs.m22 * rhs.z + lhs.m23 * rhs.w;
    ret.w = lhs.m30 * rhs.x + lhs.m31 * rhs.y + lhs.m32 * rhs.z + lhs.m33 * rhs.w;
    return ret;
}

Mat4
Mat4::LookAt(const Vec3& eye, const Vec3& center, const Vec3& up)
{
    using namespace Math;
    // The camera will look down into the negative z axis and
    // the coordinate system is right-handed.
    Vec3 z_axis = Normalize(eye - center);
    Vec3 x_axis = Normalize(Cross(up, z_axis));
    Vec3 y_axis = Cross(z_axis, x_axis);

    return Mat4(
        x_axis.x, x_axis.y, x_axis.z, -Dot(x_axis, eye),
        y_axis.x, y_axis.y, y_axis.z, -Dot(y_axis, eye),
        z_axis.x, z_axis.y, z_axis.z, -Dot(z_axis, eye),
               0,        0,        0,                 1
    );
}

Mat4
Mat4::LookAt(const Vec3& eye, const Vec3& forward, const Vec3& right, const Vec3& up)
{
    using namespace Math;
    Vec3 z_axis = -forward;
    Vec3 x_axis = right;
    Vec3 y_axis = up;

    return Mat4(
        x_axis.x, x_axis.y, x_axis.z, -Dot(x_axis, eye),
        y_axis.x, y_axis.y, y_axis.z, -Dot(y_axis, eye),
        z_axis.x, z_axis.y, z_axis.z, -Dot(z_axis, eye),
               0,        0,        0,                 1
    );
}

Mat4
Mat4::Perspective(float fovy, float aspect_ratio, float near, float far)
{
    float scale = tan(fovy * 0.5 * M_PI / 180) * near; 
    float right = aspect_ratio * scale;
    float left = -right; 
    float top = scale;
    float bottom = -top; 
    return Frustum(left, right, bottom, top, near, far);
}

Mat4 
Mat4::Frustum(float left, float right, float bottom, float top, float near, float far)
{
    float A = (right + left) / (right - left);
    float B = (top + bottom) / (top - bottom);
    float C = (2.0f * near) / (top - bottom);
    float D = (2.0f * near) / (right - left);
    float E = - (far + near) / (far - near);
    float F = - 2*(far * near)/(far - near);
    return Mat4(
           D, 0.0f,     A, 0.0f,
        0.0f,    C,     B, 0.0f,
        0.0f, 0.0f,     E,    F,
        0.0f, 0.0f, -1.0f, 0.0f
    );
}

Mat4 
Mat4::Ortho(float left, float right, float bottom, float top, float near, float far)
{
    return Mat4(
        2.0f/(right - left),                0.0f,               0.0f, - (right + left)/(right - left),
                       0.0f, 2.0f/(top - bottom),               0.0f, - (top + bottom)/(top - bottom),
                       0.0f,                0.0f, -2.0f/(far - near),     - (far + near)/(far - near),
                       0.0f,                0.0f,               0.0f,                            1.0f
    );
}