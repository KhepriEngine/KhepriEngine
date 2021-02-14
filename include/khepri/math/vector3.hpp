#pragma once

#include "vector2.hpp"

#include <gsl/gsl-lite.hpp>

#include <cassert>
#include <cmath>

namespace khepri {

class Vector4;

/**
 * \brief A 3-component vector
 */
#pragma pack(push, 1)
class Vector3 final
{
public:
    /// The type of the vector's components
    using component_type = float;

    component_type x{}; ///< The vector's X element
    component_type y{}; ///< The vector's Y element
    component_type z{}; ///< The vector's Z element

    /// Constructs an uninitialized vector
    Vector3() noexcept = default;

    /// Constructs the vector from literals
    Vector3(component_type fx, component_type fy, component_type fz) noexcept : x(fx), y(fy), z(fz)
    {}

    /// Constructs the vector from a vector2, and a Z
    Vector3(const Vector2& v, component_type fz) noexcept : x(v.x), y(v.y), z(fz) {}

    /// Constructs the vector from a Vector4 by throwing away the W component
    explicit Vector3(const Vector4& v) noexcept;

    /// Adds vector \a v to the vector
    Vector3& operator+=(const Vector3& v) noexcept
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    /// Subtracts vector \a v from the vector
    Vector3& operator-=(const Vector3& v) noexcept
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    /// Scales the vector by scalar \a s
    Vector3& operator*=(component_type s) noexcept
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    /// Scales the vector with scalar 1 / \a s
    Vector3& operator/=(component_type s) noexcept
    {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    /**
     * Indexes the vector.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y, etc.
     */
    const component_type& operator[](std::size_t index) const noexcept
    {
        assert(index < 3);
        return gsl::span<const component_type>(&x, 3)[index];
    }

    /**
     * Indexes the vector.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y, etc.
     */
    component_type& operator[](std::size_t index) noexcept
    {
        assert(index < 3);
        return gsl::span<component_type>(&x, 3)[index];
    }

    /// Calculates the length of the vector
    [[nodiscard]] component_type length() const noexcept
    {
        return sqrt(x * x + y * y + z * z);
    }

    /**
     * \brief Calculates the squared length of the vector
     *
     * Calculating the squared length (length*length) is a considerably faster operation so use it
     * whenever possible (e.g., when comparing lengths)
     */
    [[nodiscard]] component_type length_sq() const noexcept
    {
        return x * x + y * y + z * z;
    }

    /// Calculates the distance between the vector and vector \a v
    [[nodiscard]] component_type distance(const Vector3& v) const noexcept
    {
        const auto dx = v.x - x;
        const auto dy = v.y - y;
        const auto dz = v.z - z;
        return sqrt(dx * dx + dy * dy + dz * dz);
    }

    /**
     * \brief Calculates the squared distance between the vector and vector \a v
     *
     * Calculating the squared distance (distance*distance) is a considerably faster operation so
     * use it whenever possible (e.g., when comparing distances)
     */
    [[nodiscard]] component_type distance_sq(const Vector3& v) const noexcept
    {
        const auto dx = v.x - x;
        const auto dy = v.y - y;
        const auto dz = v.z - z;
        return dx * dx + dy * dy + dz * dz;
    }

    /// Calculates the dot product between the vector and vector \a v
    [[nodiscard]] component_type dot(const Vector3& v) const noexcept
    {
        return x * v.x + y * v.y + z * v.z;
    }

    /// Interprets the vector as a point on a sphere and returns the tilt (angle above the XY
    /// plane), in radians
    [[nodiscard]] component_type tilt() const noexcept
    {
        return atan2(z, sqrt(x * x + y * y));
    }

    /// Interprets the vector as a point on a sphere and returns the Z-angle (angle around Z-axis),
    /// in radians \note 0 radians is the X axis and 1/2 PI radians is the Y axis
    [[nodiscard]] component_type z_angle() const noexcept
    {
        return atan2(y, x);
    }

    /// Calculates the cross product between the vector and vector \a v
    [[nodiscard]] Vector3 cross(const Vector3& v) const noexcept
    {
        return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }

    /// Normalizes the vector
    void normalize() noexcept
    {
        component_type inv_length = 1.0F / length();
        x *= inv_length;
        y *= inv_length;
        z *= inv_length;
    }

    /// Checks if the vector is normalized
    [[nodiscard]] bool normalized() const noexcept
    {
        constexpr auto max_normalized_length = 0.000001;
        return abs(1.0F - length()) < max_normalized_length;
    }
};
#pragma pack(pop)

/// Validate that the vector has the expected size, because this type can be directly used in a
/// mapping to graphics engine's memory.
static_assert(sizeof(Vector3) == 3 * sizeof(Vector3::component_type),
              "Vector3 does not have the expected size");

/// Negates vector \a v
inline Vector3 operator-(const Vector3& v) noexcept
{
    return Vector3(-v.x, -v.y, -v.z);
}

/// Adds vector \a v2 to vector \a v1
inline Vector3 operator+(const Vector3& v1, const Vector3& v2) noexcept
{
    return Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

/// Subtracts vector \a v2 from vector \a v1
inline Vector3 operator-(const Vector3& v1, const Vector3& v2) noexcept
{
    return Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

/// Scales vector \a v with scalar \a s
inline Vector3 operator*(const Vector3& v, float s) noexcept
{
    return Vector3(v.x * s, v.y * s, v.z * s);
}

/// Scales vector \a v with scalar \a s
inline Vector3 operator*(float s, const Vector3& v) noexcept
{
    return Vector3(v.x * s, v.y * s, v.z * s);
}

/// Scales vector \a v with scalar 1/\a s
inline Vector3 operator/(const Vector3& v, float s) noexcept
{
    return Vector3(v.x / s, v.y / s, v.z / s);
}

/// Multiplies vector \a v1 with vector \a v2, component-wise
inline Vector3 operator*(const Vector3& v1, const Vector3& v2) noexcept
{
    return Vector3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

/// Calculates the distance between the points identified by vector \a v1 and vector \a v2
inline Vector3::component_type distance(const Vector3& v1, const Vector3& v2) noexcept
{
    return v1.distance(v2);
}

/**
 * \brief Calculates the squared distance between the points identified by vector \a v1 and vector
 *        \a v2
 *
 * Calculating the squared distance (distance*distance) is a considerably faster operation so
 * use it whenever possible (e.g., when comparing distances)
 */
inline Vector3::component_type distance_sq(const Vector3& v1, const Vector3& v2) noexcept
{
    return v1.distance_sq(v2);
}

/// Calculates the dot product between vector \a v1 and vector \a v2
inline Vector3::component_type dot(const Vector3& v1, const Vector3& v2) noexcept
{
    return v1.dot(v2);
}

/// Calculates the cross product between vector \a v1 and vector \a v2
inline Vector3 cross(const Vector3& v1, const Vector3& v2) noexcept
{
    return v1.cross(v2);
}

/// Normalizes vector \a v
inline Vector3 normalize(const Vector3& v) noexcept
{
    Vector3 nv(v);
    nv.normalize();
    return nv;
}

inline Vector2::Vector2(const Vector3& v) noexcept : x(v.x), y(v.y) {}

/**
 * Checks if two vectors are colinear.
 *
 * Two vectors are colinear if they point in the same direction. Opposite vectors are also
 * considered colinear.
 *
 * \note as with all floating-point checks, there is a tiny error margin
 */
inline bool colinear(const Vector3& v1, const Vector3& v2) noexcept
{
    constexpr auto max_colinear_sq_length = 0.000001;
    return cross(v1, v2).length_sq() < max_colinear_sq_length;
}

} // namespace khepri
