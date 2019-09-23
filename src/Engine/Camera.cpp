#include "Han/Camera.hpp"

Camera::Camera(
	Vec3 position, 
	Vec3 front, 
	float aspect_ratio, 
	float fov, 
	float base_move_speed, 
	float base_rotation_speed, 
	float near,
	float far
)
    : position(position)
    , front(0, Math::Normalize(front))
    , up_world(0, 1, 0)
    , projection_matrix(Mat4::Perspective(fov, aspect_ratio, near, far))
	, base_move_speed(base_move_speed)
	, base_rotation_speed(base_rotation_speed)
{
    UpdateUpAndRightVectors();
}

Mat4
Camera::GetViewMatrix()
{
    using namespace Math;
    const float epsilon = std::numeric_limits<float>::epsilon();

    Vec3 up_world;
    if (fabs(front.v.x) < epsilon && fabs(front.v.z) < epsilon) {
        if (front.v.y > 0)
            up_world = Vec3(0, 0, -1);
        else
            up_world = Vec3(0, 0, 1);
    } else {
        up_world = Vec3(0, 1, 0);
    }

    right = Normalize(Cross(front.v, up_world));
    auto view_matrix = Mat4::LookAt(position, front.v, right, up);
    return view_matrix;
}

Mat4
Camera::GetViewProjectionMatrix(const Mat4& view)
{
    return projection_matrix * view;
}

void
Camera::Rotate(const Vec3& axis, float rotation_speed)
{
    using namespace Math;
    // HACK, TODO: remove this GAMBETA

    front = Quaternion::Rotate(front, rotation_speed, Quaternion(0, axis));

    UpdateUpAndRightVectors();
}

void
Camera::UpdateUpAndRightVectors()
{
    using namespace Math;
    const float epsilon = std::numeric_limits<float>::epsilon();

    if (fabs(front.v.x) < epsilon && fabs(front.v.z) < epsilon) {
        if (front.v.y > 0) {
            up_world = Vec3(0, 0, 1);
        } else {
            up_world = Vec3(0, 0, -1);
        }
    } else {
        up_world = Vec3(0, 1, 0);
    }

    right = Normalize(Cross(front.v, up_world));
    up = Normalize(Cross(right, front.v));
}

void
Camera::Update(DeltaTime delta_time)
{
	move_speed = base_move_speed * delta_time;
	rotation_speed = base_rotation_speed * delta_time;
}
