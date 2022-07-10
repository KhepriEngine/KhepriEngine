#pragma once

#include <cmath>
#include <stdexcept>

namespace khepri {

class Vector3;
class Vector4;
class Quaternion;

Quaternion operator*(const Quaternion& q1, const Quaternion& q2) noexcept;

/**
 * \brief Quaternion
 */
#pragma pack(push, 1)
class Quaternion final
{
public:
    /// The type of the quaternion's components
    using ComponentType = float;

    ComponentType x; ///< The quaternion's X element
    ComponentType y; ///< The quaternion's Y element
    ComponentType z; ///< The quaternion's Z element
    ComponentType w; ///< The quaternion's W element

    /// Constructs an uninitialized quaternion
    Quaternion() noexcept = default;

    /// Constructs a quaternion from immediate floats
    Quaternion(ComponentType fx, ComponentType fy, ComponentType fz, ComponentType fw) noexcept
        : x(fx), y(fy), z(fz), w(fw)
    {}

    /// Adds another quaternion to this quaternion
    Quaternion& operator+=(const Quaternion& q) noexcept
    {
        x += q.x;
        y += q.y;
        z += q.z;
        w += q.w;
        return *this;
    }

    /// Subtracts another quaternion from this quaternion
    Quaternion& operator-=(const Quaternion& q) noexcept
    {
        x -= q.x;
        y -= q.y;
        z -= q.z;
        w -= q.w;
        return *this;
    }

    /// Multiplies another quaternion with this quaternion
    Quaternion& operator*=(const Quaternion& q) noexcept
    {
        return *this = *this * q;
    }

    /// Scales the quaternion
    Quaternion& operator*=(ComponentType s) noexcept
    {
        x *= s, y *= s, z *= s, w *= s;
        return *this;
    }

    /// Scales the quaternion (inverted)
    Quaternion& operator/=(ComponentType s) noexcept
    {
        x /= s, y /= s, z /= s, w /= s;
        return *this;
    }

    /**
     * Indexes the quaternion.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y, etc.
     *
     * \throws std::out_of_range if the index is not in the range [0,3].
     */
    const float& operator[](std::size_t index) const
    {
        switch (index) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        case 3:
            return w;
        }
        throw std::out_of_range("invalid Quaternion subscript");
    }

    /**
     * Indexes the quaternion.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y, etc.
     *
     * \throws std::out_of_range if the index is not in the range [0,3].
     */
    float& operator[](std::size_t index)
    {
        switch (index) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        case 3:
            return w;
        }
        throw std::out_of_range("invalid Quaternion subscript");
    }

    /// Normalizes the quaternion
    void normalize() noexcept
    {
        const ComponentType inv_length = ComponentType(1.0) / length();
        x *= inv_length;
        y *= inv_length;
        z *= inv_length;
        w *= inv_length;
    }

    /// Checks if the quaternion is normalized
    [[nodiscard]] bool normalized() const noexcept
    {
        constexpr auto max_normalized_length = 0.000001;
        return abs(ComponentType(1.0) - length()) < max_normalized_length;
    }

    /// Calculates the length of the quaternion
    [[nodiscard]] ComponentType length() const noexcept
    {
        return sqrt(x * x + y * y + z * z + w * w);
    }

    /**
     \brief Calculates the squared length of the quaternion
     \details Calculating the squared length (length*length) is a considerably faster operation
        so use it whenever possible (e.g., when comparing lengths)
     */
    [[nodiscard]] ComponentType length_sq() const noexcept
    {
        return x * x + y * y + z * z + w * w;
    }

    /// Calculates the dot product between this and the specified quaternion
    [[nodiscard]] ComponentType dot(const Quaternion& v) const noexcept
    {
        return x * v.x + y * v.y + z * v.z + w * v.w;
    }

    /// Converts the quaternion to a Euler rotation representation
    [[nodiscard]] Vector3 to_euler() const noexcept;

    /**
     * \brief Constructs a quaternion to represent a rotation around an axis
     */
    static Quaternion
    from_axis_angle(const Vector3& axis, ///< [in] The axis to rotate around
                    ComponentType angle ///< [in] The angle, in radians, to rotate around the axis
                    ) noexcept;

    /**
     * \brief Constructs a quaternion from Euler rotation angles
     *
     * The applied rotation order is: First rotation around X-axis, then Y-axis, then Z-axis.
     */
    static Quaternion from_euler(ComponentType x, ///< [in] Rotation, in radians, around the X-axis
                                 ComponentType y, ///< [in] Rotation, in radians, around the Y-axis
                                 ComponentType z  ///< [in] Rotation, in radians, around the Z-axis
                                 ) noexcept;

    /// Identity quaternion
    static const Quaternion IDENTITY;
};
#pragma pack(pop)

/// Validate that the vector has the expected size, because this type can be directly used in a
/// mapping to graphics engine's memory.
static_assert(sizeof(Quaternion) == 4 * sizeof(Quaternion::ComponentType),
              "Quaternion does not have the expected size");

/// Adds quaternion \a q2 to quaternion \a q1
inline Quaternion operator+(const Quaternion& q1, const Quaternion& q2) noexcept
{
    return Quaternion(q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w);
}

/// Subtracts quaternion \a q2 from quaternion \a q1
inline Quaternion operator-(const Quaternion& q1, const Quaternion& q2) noexcept
{
    return Quaternion(q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w);
}

/// Scales quaternion \a q with scalar \a s
inline Quaternion operator*(const Quaternion& q, float s) noexcept
{
    return Quaternion(q.x * s, q.y * s, q.z * s, q.w * s);
}

/// Scales quaternion \a q with scalar \a s
inline Quaternion operator*(float s, const Quaternion& q) noexcept
{
    return Quaternion(q.x * s, q.y * s, q.z * s, q.w * s);
}

/// Scales quaternion \a q with scalar 1/\a s
inline Quaternion operator/(const Quaternion& q, float s) noexcept
{
    return Quaternion(q.x / s, q.y / s, q.z / s, q.w / s);
}

/// Computes the dot-product of quaternion \a q1 and quaternion \a q2
inline Quaternion::ComponentType dot(const Quaternion& q1, const Quaternion& q2) noexcept
{
    return q1.dot(q2);
}

/// Normalizes quaternion \a q
inline Quaternion normalize(const Quaternion& q) noexcept
{
    const float inv_length = 1.0F / q.length();
    return Quaternion(q.x * inv_length, q.y * inv_length, q.z * inv_length, q.w * inv_length);
}

/// Transforms (post-multiplies) a vector with a rotation quaternion
Vector3 operator*(const Vector3& v, const Quaternion& q) noexcept;

/// Transforms (post-multiplies) a vector with a rotation quaternion
Vector4 operator*(const Vector4& v, const Quaternion& q) noexcept;

/**
 * \brief spherical linear interpolation between quaternions.
 *
 * \note for performance reasons this implementation does not do true spherical linear
 * implementation, but instead does linear interpolation. This will generally make rotations look
 * good enough.
 */
Quaternion slerp(const Quaternion& v0, const Quaternion& v1, float t) noexcept;

/// Computes the inverse of quaternion \a q
inline Quaternion inverse(const Quaternion& q) noexcept
{
    const float inv_length = 1.0F / q.length_sq();
    return Quaternion(-q.x, -q.y, -q.z, q.w) * inv_length;
}

} // namespace khepri
