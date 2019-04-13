#include "Math/Mat4.hpp"

Mat4
operator*(const Mat4& lhs, const Mat4& rhs)
{
    Mat4 ret;
    //------------------------------
    // First row
    //------------------------------
    //  m00 = dot(lhs.row0, rhs.col0)
    ret.m00 = lhs.m00*rhs.m00 + lhs.m01*rhs.m10 + lhs.m02*rhs.m20 + lhs.m03*rhs.m30;
    //  m01 = dot(lhs.row0, rhs.col1)
    ret.m01 = lhs.m00*rhs.m01 + lhs.m01*rhs.m11 + lhs.m02*rhs.m21 + lhs.m03*rhs.m31;
    //  m02 = dot(lhs.row0, rhs.col2)
    ret.m02 = lhs.m00*rhs.m02 + lhs.m01*rhs.m12 + lhs.m02*rhs.m22 + lhs.m03*rhs.m32;
    //  m03 = dot(lhs.row0, rhs.col3)
    ret.m03 = lhs.m00*rhs.m03 + lhs.m01*rhs.m13 + lhs.m02*rhs.m23 + lhs.m03*rhs.m33;
    //------------------------------
    // Second row
    //------------------------------
    //  m10 = dot(lhs.row1, rhs.col0)
    ret.m10 = lhs.m10*rhs.m00 + lhs.m11*rhs.m10 + lhs.m12*rhs.m20 + lhs.m13*rhs.m30;
    //  m11 = dot(lhs.row1, rhs.col1)
    ret.m11 = lhs.m10*rhs.m01 + lhs.m11*rhs.m11 + lhs.m12*rhs.m21 + lhs.m13*rhs.m31;
    //  m12 = dot(lhs.row1, rhs.col2)
    ret.m12 = lhs.m10*rhs.m02 + lhs.m11*rhs.m12 + lhs.m12*rhs.m22 + lhs.m13*rhs.m32;
    //  m13 = dot(lhs.row1, rhs.col3)
    ret.m13 = lhs.m10*rhs.m03 + lhs.m11*rhs.m13 + lhs.m12*rhs.m23 + lhs.m13*rhs.m33;
    //------------------------------
    // Third row
    //------------------------------
    //  m20 = dot(lhs.row2, rhs.col0)
    ret.m20 = lhs.m20*rhs.m00 + lhs.m21*rhs.m10 + lhs.m22*rhs.m20 + lhs.m23*rhs.m30;
    //  m21 = dot(lhs.row2, rhs.col1)
    ret.m21 = lhs.m20*rhs.m01 + lhs.m21*rhs.m11 + lhs.m22*rhs.m21 + lhs.m23*rhs.m31;
    //  m22 = dot(lhs.row2, rhs.col2)
    ret.m22 = lhs.m20*rhs.m02 + lhs.m21*rhs.m12 + lhs.m22*rhs.m22 + lhs.m23*rhs.m32;
    //  m23 = dot(lhs.row2, rhs.col3)
    ret.m23 = lhs.m20*rhs.m03 + lhs.m21*rhs.m13 + lhs.m22*rhs.m23 + lhs.m23*rhs.m33;
    //------------------------------
    // Fourth row
    //------------------------------
    //  m30 = dot(lhs.row3, rhs.col0)
    ret.m30 = lhs.m30*rhs.m00 + lhs.m31*rhs.m10 + lhs.m32*rhs.m20 + lhs.m33*rhs.m30;
    //  m31 = dot(lhs.row3, rhs.col1)
    ret.m31 = lhs.m30*rhs.m01 + lhs.m31*rhs.m11 + lhs.m32*rhs.m21 + lhs.m33*rhs.m31;
    //  m32 = dot(lhs.row3, rhs.col2)
    ret.m32 = lhs.m30*rhs.m02 + lhs.m31*rhs.m12 + lhs.m32*rhs.m22 + lhs.m33*rhs.m32;
    //  m33 = dot(lhs.row3, rhs.col3)
    ret.m33 = lhs.m30*rhs.m03 + lhs.m31*rhs.m13 + lhs.m32*rhs.m23 + lhs.m33*rhs.m33;
    return ret;
}

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
