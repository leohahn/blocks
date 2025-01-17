#pragma once

#include "Han/Math/Vec3.hpp"
#include "Han/Math/Mat4.hpp"
#include "Han/Math/Quaternion.hpp"
#include "Han/Core.hpp"

struct Camera
{
public:
    Vec3 position;
    Quaternion front;
    Vec3 up;
    Vec3 right;
    Vec3 up_world;
    Mat4 projection_matrix;
	const float base_move_speed;
	const float base_rotation_speed;
	float move_speed;
	float rotation_speed;
   
public:
    Camera(
		Vec3 position, 
		Vec3 front, 
		float aspect_ratio, 
		float fov, 
		float base_move_speed, 
		float base_rotation_speed, 
		float near = 0.1f, 
		float far = 100.0f
	);
    Mat4 GetViewMatrix();
    Mat4 GetViewProjectionMatrix(const Mat4& view);
	void Update(DeltaTime delta_time);
    //
    // HACK, TODO: remove this GAMBETA
    //
#if OS_APPLE
    void MoveLeft(float offset) { position -= right * offset; }
#else
    void MoveLeft(float offset) { position -= right * offset; }
#endif
    
#if OS_APPLE
    void MoveRight(float offset) { position += right * offset; }
#else
    void MoveRight(float offset) { position += right * offset; }
#endif
    
#if OS_APPLE
    void MoveForwards(float offset) { position += front.v * offset; }
#else
    void MoveForwards(float offset) { position += front.v * offset; }
#endif

#if OS_APPLE
    void MoveBackwards(float offset) { position -= front.v * offset; }
#else
    void MoveBackwards(float offset) { position -= front.v * offset; }
#endif
    void Rotate(const Vec3& axis, float rotation_speed);

private:
    void UpdateUpAndRightVectors();
};
