#ifndef BLOCKS_MAT4_HPP
#define BLOCKS_MAT4_HPP

// A translation is layed out in memory like this:
// { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, transX, transY, transZ, 1 }.
struct Mat4
{
    // Matrix data
    union
    {
        float m[4][4];
        float data[16];
    };

public:
    // Matrix API

    static Mat4 New(float m00, float m01, float m02, float m03,
                    float m10, float m11, float m12, float m13,
                    float m20, float m21, float m22, float m23,
                    float m30, float m31, float m32, float m33)
    {
        Mat4 mat;
        mat.data[0]  = m00;
        mat.data[1]  = m10;
        mat.data[2]  = m20;
        mat.data[3]  = m30;
        mat.data[4]  = m01;
        mat.data[5]  = m11;
        mat.data[6]  = m21;
        mat.data[7]  = m31;
        mat.data[8]  = m02;
        mat.data[9]  = m12;
        mat.data[10] = m22;
        mat.data[11] = m32;
        mat.data[12] = m03;
        mat.data[13] = m13;
        mat.data[14] = m23;
        mat.data[15] = m33;
        return mat;
    }

    static Mat4 Diag(float diag)
    {
        return New(diag, 0,    0,    0,
                   0,    diag, 0,    0,
                   0,    0,    diag, 0,
                   0,    0,    0,    1);
    }

    static Mat4 LookAt()
    {
        // TODO: Implement
    }

    static Mat4 Perspective()
    {
        // TODO: Implement
    }

    static Mat4 Ortho()
    {
        // TODO: Implement
    }
};

#endif // BLOCKS_MAT4_HPP
