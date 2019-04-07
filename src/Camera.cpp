#include "Camera.hpp"

Camera::Camera(Vec3 position, Vec3 front)
    : position(position)
    , up_world(0, 1, 0)
    , front(0, Math::Normalize(front))
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

void
Camera::Rotate(const Vec3& axis)
{
    using namespace Math;
    float rotation_speed = 0.001f;

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
