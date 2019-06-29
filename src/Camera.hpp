#pragma once

#include "Math/Vec3.hpp"
#include "Math/Mat4.hpp"
#include "Math/Quaternion.hpp"
#include "Defines.hpp"

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
    //
    // HACK, TODO: remove this GAMBETA
    //
#if OS_APPLE
    void MoveLeft(float offset) { position -= right * 0.80f; }
#else
    void MoveLeft(float offset) { position -= right * 0.01f; }
#endif
    
#if OS_APPLE
    void MoveRight(float offset) { position += right * 0.80f; }
#else
    void MoveRight(float offset) { position += right * 0.01f; }
#endif
    
#if OS_APPLE
    void MoveForwards(float offset) { position += front.v * 0.80f; }
#else
    void MoveForwards(float offset) { position += front.v * 0.01f; }
#endif

#if OS_APPLE
    void MoveBackwards(float offset) { position -= front.v * 0.80f; }
#else
    void MoveBackwards(float offset) { position -= front.v * 0.01f; }
#endif
    void Rotate(const Vec3& axis);

private:
    void UpdateUpAndRightVectors();
};
