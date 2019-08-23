#pragma once

#include "Han/Math/Vec3.hpp"
#include "Han/Math/Vec4.hpp"

// A matrix is represented in this class
// as column major:
// | right_x up_x forward_x pos_x |
// | right_y up_y forward_y pos_y |
// | right_z up_z forward_z pos_z |
// |       0    0         0     1 |

// A translation is layed out in memory like this:
// { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, transX, transY, transZ, 1 }.
struct Mat4
{
    // Matrix data
    union
    {
        struct
        {
            float m00, m10, m20, m30;
            float m01, m11, m21, m31;
            float m02, m12, m22, m32;
            float m03, m13, m23, m33;
        };
        float m[4][4];
        float data[16];
    };

public:
    // Matrix API
    float& operator()(int row, int col) { return m[col][row]; }

    Mat4() = default;

    // clang-format off
    Mat4(float m00, float m01, float m02, float m03,
         float m10, float m11, float m12, float m13,
         float m20, float m21, float m22, float m23,
         float m30, float m31, float m32, float m33)
    {
        data[0]  = m00;
        data[1]  = m10;
        data[2]  = m20;
        data[3]  = m30;
        data[4]  = m01;
        data[5]  = m11;
        data[6]  = m21;
        data[7]  = m31;
        data[8]  = m02;
        data[9]  = m12;
        data[10] = m22;
        data[11] = m32;
        data[12] = m03;
        data[13] = m13;
        data[14] = m23;
        data[15] = m33;
    }

    static Mat4 Zero()
    {
        return Mat4(0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0);
    }

    static Mat4 Identity()
    {
        return Mat4(1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1);
    }

    static Mat4 Scale(float scale)
    {
        return Mat4(scale, 0,    0,    0,
                    0,    scale, 0,    0,
                    0,    0,    scale, 0,
                    0,    0,     0,    1);
    }
    // clang-format on

    static Mat4 LookAt(const Vec3& eye, const Vec3& center, const Vec3& up_world);

    static Mat4 LookAt(const Vec3& eye, const Vec3& forward, const Vec3& right, const Vec3& up);

    static Mat4 Frustum(float left, float right, float bottom, float top, float near, float far);

    static Mat4 Perspective(float fovy, float aspect_ratio, float near, float far);

    static Mat4 Ortho(float left, float right, float bottom, float top, float near, float far);
};

Mat4 operator*(const Mat4& lhs, const Mat4& rhs);
Vec4 operator*(const Mat4& lhs, const Vec4& rhs);
