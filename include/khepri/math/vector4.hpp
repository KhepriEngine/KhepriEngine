#pragma once

#include "vector3.hpp"

#include <gsl/gsl-lite.hpp>

#include <cassert>
#include <cmath>

namespace khepri {

class matrix;

/// \brief A 4-component vector
#pragma pack(push, 0)
class Vector4 final
{
public:
    /// The type of the vector's components
    using component_type = float;

    component_type x{}; ///< The vector's X element
    component_type y{}; ///< The vector's Y element
    component_type z{}; ///< The vector's Z element
    component_type w{}; ///< The vector's W element

    /// Constructs an uninitialized vector
    constexpr Vector4() noexcept = default;

    /// Constructs the vector from literal floats
    constexpr Vector4(component_type fx, component_type fy, component_type fz,
                      component_type fw) noexcept
        : x(fx), y(fy), z(fz), w(fw)
    {}

    /// Constructs the vector from a vector2, and floats for Z and W
    constexpr Vector4(const Vector2& v, component_type fz, component_type fw) noexcept
        : x(v.x), y(v.y), z(fz), w(fw)
    {}

    /// Constructs the vector from a vector3, and a float for W
    constexpr Vector4(const Vector3& v, component_type fw) noexcept : x(v.x), y(v.y), z(v.z), w(fw)
    {}

    /// Adds vector \a v to the vector
    constexpr Vector4& operator+=(const Vector4& v) noexcept
    {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;
        return *this;
    }

    /// Subtracts vector \a v from the vector
    constexpr Vector4& operator-=(const Vector4& v) noexcept
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;
        return *this;
    }

    /// Scales the vector by scalar \a s
    constexpr Vector4& operator*=(component_type s) noexcept
    {
        x *= s;
        y *= s;
        z *= s;
        w *= s;
        return *this;
    }

    /// Scales the vector by scalar 1.0 / \a s
    constexpr Vector4& operator/=(component_type s) noexcept
    {
        x /= s;
        y /= s;
        z /= s;
        w /= s;
        return *this;
    }

    /**
     * Indexes the vector.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y, etc.
     */
    constexpr const component_type& operator[](std::size_t index) const noexcept
    {
        assert(index < 4);
        return gsl::span<const component_type>(&x, 4)[index];
    }

    /**
     * Indexes the vector.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y, etc.
     */
    constexpr component_type& operator[](std::size_t index) noexcept
    {
        assert(index < 4);
        return gsl::span<component_type>(&x, 4)[index];
    }

    /// Calculates the length of the vector
    [[nodiscard]] component_type length() const noexcept
    {
        return std::sqrt(length_sq());
    }

    /**
     * \brief Calculates the squared length of the vector
     *
     * Calculating the squared length (length*length) is a considerably faster operation so use it
     * whenever possible (e.g., when comparing lengths)
     */
    [[nodiscard]] constexpr component_type length_sq() const noexcept
    {
        return x * x + y * y + z * z + w * w;
    }

    /// Calculates the distance between the vector and vector \a v
    [[nodiscard]] component_type distance(const Vector4& v) const noexcept
    {
        const auto dx = v.x - x;
        const auto dy = v.y - y;
        const auto dz = v.z - z;
        const auto dw = v.w - w;
        return std::sqrt(dx * dx + dy * dy + dz * dz + dw * dw);
    }

    /**
     * \brief Calculates the squared distance between the vector and vector \a v
     *
     * Calculating the squared distance (distance*distance) is a considerably faster operation so
     * use it whenever possible (e.g., when comparing distances)
     */
    [[nodiscard]] constexpr component_type distance_sq(const Vector4& v) const noexcept
    {
        const auto dx = v.x - x;
        const auto dy = v.y - y;
        const auto dz = v.z - z;
        const auto dw = v.w - w;
        return dx * dx + dy * dy + dz * dz + dw * dw;
    }

    /// Calculates the dot product between the vector and vector \a v
    [[nodiscard]] constexpr component_type dot(const Vector4& v) const noexcept
    {
        return x * v.x + y * v.y + z * v.z + w * v.w;
    }

    /// Normalizes the vector
    constexpr void normalize() noexcept
    {
        const component_type inv_length = 1.0F / length();
        x *= inv_length;
        y *= inv_length;
        z *= inv_length;
        w *= inv_length;
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
static_assert(sizeof(Vector4) == 4 * sizeof(Vector4::component_type),
              "Vector4 does not have the expected size");

/// Negates vector \a v
inline constexpr Vector4 operator-(const Vector4& v) noexcept
{
    return Vector4(-v.x, -v.y, -v.z, -v.w);
}

/// Adds vector \a v2 to vector \a v1
inline constexpr Vector4 operator+(const Vector4& v1, const Vector4& v2) noexcept
{
    return Vector4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

/// Subtracts vector \a v2 from vector \a v1
inline constexpr Vector4 operator-(const Vector4& v1, const Vector4& v2) noexcept
{
    return Vector4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

/// Scales vector \a v with scalar \a s
inline constexpr Vector4 operator*(const Vector4& v, float s) noexcept
{
    return Vector4(v.x * s, v.y * s, v.z * s, v.w * s);
}

/// Scales vector \a v with scalar \a s
inline constexpr Vector4 operator*(float s, const Vector4& v) noexcept
{
    return Vector4(v.x * s, v.y * s, v.z * s, v.w * s);
}

/// Scales vector \a v with scalar 1/\a s
inline constexpr Vector4 operator/(const Vector4& v, float s) noexcept
{
    return Vector4(v.x / s, v.y / s, v.z / s, v.w / s);
}

/// Multiplies vector \a v1 with vector \a v2, component-wise
inline constexpr Vector4 operator*(const Vector4& v1, const Vector4& v2) noexcept
{
    return Vector4(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
}

/// Calculates the distance between the points identified by vector \a v1 and vector \a v2
inline constexpr float distance(const Vector4& v1, const Vector4& v2) noexcept
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
inline constexpr float distance_sq(const Vector4& v1, const Vector4& v2) noexcept
{
    return v1.distance_sq(v2);
}

/// Calculates the dot product between vector \a v1 and vector \a v2
inline constexpr float dot(const Vector4& v1, const Vector4& v2) noexcept
{
    return v1.dot(v2);
}

/// Normalizes vector \a v
inline Vector4 normalize(const Vector4& v) noexcept
{
    return v / v.length();
}

inline Vector2::Vector2(const Vector4& v) noexcept : x(v.x), y(v.y) {}
inline Vector3::Vector3(const Vector4& v) noexcept : x(v.x), y(v.y), z(v.z) {}

/**
 * \brief Linearly interpolates between vector \a v0 and vector \a v1 based on factor \a t.
 *
 * \a t is assumed to be between 0 and 1. If \a t is 0, \a v0 is returned. If \a t is 1, \a v1 is
 * returned. It is valid for \a t to be outside of this range, in which case the result is an
 * extrapolation.
 */
inline constexpr Vector4 lerp(const Vector4& v0, const Vector4& v1, float t) noexcept
{
    return v1 * t + v0 * (1.0F - t);
}

} // namespace khepri
