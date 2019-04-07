#pragma once

#include "Math/Vec3.hpp"
#include "Math/Mat4.hpp"
#include "Math/Quaternion.hpp"

struct Camera
{
public:
    Vec3 position;
    Quaternion front;
    Vec3 up;
    Vec3 right;
    Vec3 up_world;
   
public:
    Camera(Vec3 position, Vec3 front);
    Mat4 GetViewMatrix();
    void MoveLeft(float offset) { position -= right * 0.01f; }
    void MoveRight(float offset) { position += right * 0.01f; }
    void MoveForwards(float offset) { position += front.v * 0.01f; }
    void MoveBackwards(float offset) { position -= front.v * 0.01f; }
    void Rotate(const Vec3& axis);

private:
    void UpdateUpAndRightVectors();
};
