#include <khepri/math/quaternion.hpp>
#include <khepri/math/vector3.hpp>
#include <khepri/math/vector4.hpp>

namespace khepri {

const Quaternion Quaternion::IDENTITY(0, 0, 0, 1);

Vector3 Quaternion::to_euler() const noexcept
{
    return Vector3(-std::atan2(-2 * (y * z - w * x), 1 - 2 * (x * x + y * y)),
                   -std::asin(2 * (x * z + w * y)),
                   -std::atan2(-2 * (x * y - w * z), 1 - 2 * (y * y + z * z)));
}

Quaternion operator*(const Quaternion& q1, const Quaternion& q2) noexcept
{
    return Quaternion(q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
                      q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
                      q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
                      q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z);
}

Vector3 operator*(const Vector3& v, const Quaternion& q) noexcept
{
    // Optimized version of Matrix(q).transform_coord(v)
    const Vector3 qv(q.x, q.y, q.z);
    const Vector3 t = qv.cross(v) * 2;
    return v + t * q.w + qv.cross(t);
}

Vector4 operator*(const Vector4& v, const Quaternion& q) noexcept
{
    // Apply the transformation to the XYZ components of the vector and leave W untouched.
    return Vector4(Vector3(v) * q, v.w);
}

Quaternion Quaternion::from_axis_angle(const Vector3& axis, float angle) noexcept
{
    // Divide by axis' length to normalize it
    const float s = std::sin(angle / 2) / axis.length();
    return Quaternion(axis.x * s, axis.y * s, axis.z * s, cos(angle / 2));
}

// Rotation order is XYZ
Quaternion Quaternion::from_euler(float rx, float ry, float rz) noexcept
{
    const float c1 = std::cos(-rx / 2);
    const float c2 = std::cos(-ry / 2);
    const float c3 = std::cos(-rz / 2);
    const float s1 = std::sin(-rx / 2);
    const float s2 = std::sin(-ry / 2);
    const float s3 = std::sin(-rz / 2);
    return Quaternion(s1 * c2 * c3 + c1 * s2 * s3, c1 * s2 * c3 - s1 * c2 * s3,
                      c1 * c2 * s3 + s1 * s2 * c3, c1 * c2 * c3 - s1 * s2 * s3);
}

Quaternion slerp(const Quaternion& v0, const Quaternion& v1, float t) noexcept
{
    // Not really spherical linear interpolation.
    // Just linear interpolation with a possible sign-flip so rotations
    // look good (shortest path).
    const float d = v0.dot(v1);
    return v0 * (1 - t) + v1 * t * (d < 0 ? -1.0F : 1.0F);
}

} // namespace khepri