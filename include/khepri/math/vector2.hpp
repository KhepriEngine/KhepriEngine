#pragma once

#include "math.hpp"

#include <gsl/gsl-lite.hpp>

#include <cassert>
#include <cmath>

namespace khepri {

class Vector3;
class Vector4;

/// \brief A 2-component vector
#pragma pack(push, 1)
class Vector2 final
{
public:
    /// The type of the vector's components
    using ComponentType = float;

    ComponentType x{}; ///< The vector's X element
    ComponentType y{}; ///< The vector's Y element

    /// Constructs an uninitialized vector
    constexpr Vector2() noexcept = default;

    /// Constructs the vector from literal floats
    constexpr Vector2(ComponentType fx, ComponentType fy) noexcept : x(fx), y(fy) {}

    /// Constructs the vector from a vector3 by throwing away the Z component
    explicit constexpr Vector2(const Vector3& v) noexcept;

    /// Constructs the vector from a vector4 by throwing away the Z and W components
    explicit constexpr Vector2(const Vector4& v) noexcept;

    /// Adds vector \a v to the vector
    Vector2& operator+=(const Vector2& v) noexcept
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    /// Subtracts vector \a v from the vector
    Vector2& operator-=(const Vector2& v) noexcept
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    /// Scales the vector by \a s
    Vector2& operator*=(float s) noexcept
    {
        x *= s;
        y *= s;
        return *this;
    }

    /// Scales the vector with 1.0 / \a s
    Vector2& operator/=(float s) noexcept
    {
        x /= s;
        y /= s;
        return *this;
    }

    /**
     * Indexes the vector.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y.
     */
    const ComponentType& operator[](std::size_t index) const noexcept
    {
        assert(index < 2);
        return gsl::span<const ComponentType>(&x, 2)[index];
    }

    /**
     * Indexes the vector.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y.
     */
    ComponentType& operator[](std::size_t index) noexcept
    {
        assert(index < 2);
        return gsl::span<ComponentType>(&x, 2)[index];
    }

    /// Calculates the length of the vector
    [[nodiscard]] ComponentType length() const noexcept
    {
        return sqrt(x * x + y * y);
    }

    /**
     * \brief Calculates the squared length of the vector
     *
     * Calculating the squared length (length*length) is a considerably faster operation so use it
     * whenever possible (e.g., when comparing lengths)
     */
    [[nodiscard]] ComponentType length_sq() const noexcept
    {
        return x * x + y * y;
    }

    /// Calculates the distance between the vector and vector \a v
    [[nodiscard]] ComponentType distance(const Vector2& v) const noexcept
    {
        const auto dx = v.x - x;
        const auto dy = v.y - y;
        return sqrt(dx * dx + dy * dy);
    }

    /**
     * \brief Calculates the squared distance between the vector and vector \a v
     *
     * Calculating the squared distance (distance*distance) is a considerably faster operation so
     * use it whenever possible (e.g., when comparing distances)
     */
    [[nodiscard]] ComponentType distance_sq(const Vector2& v) const noexcept
    {
        const auto dx = v.x - x;
        const auto dy = v.y - y;
        return dx * dx + dy * dy;
    }

    /// Calculates the angle that the vector makes with the positive X axis
    [[nodiscard]] ComponentType angle() const noexcept
    {
        return atan2(y, x);
    }

    /// Calculates the dot product between the vector and vector \a v
    [[nodiscard]] ComponentType dot(const Vector2& v) const noexcept
    {
        return x * v.x + y * v.y;
    }

    /// Calculates the cross product between the vector and vector \a v
    [[nodiscard]] ComponentType cross(const Vector2& v) const noexcept
    {
        return x * v.y - y * v.x;
    }

    /// Normalizes the vector
    void normalize() noexcept
    {
        ComponentType inv_length = 1.0F / length();
        x *= inv_length;
        y *= inv_length;
    }

    /// Checks if the vector is normalized
    [[nodiscard]] bool normalized() const noexcept
    {
        constexpr auto max_normalized_length = 0.000001;
        return abs(1.0F - length()) < max_normalized_length;
    }

    /// Constructs a unit vector from an angle with the positive X-axis
    static Vector2 from_angle(float phi) noexcept
    {
        return Vector2(cos(phi), sin(phi));
    }
};
#pragma pack(pop)

/// Validate that the vector has the expected size, because this type can be directly used in a
/// mapping to graphics engine's memory.
static_assert(sizeof(Vector2) == 2 * sizeof(Vector2::ComponentType),
              "Vector2 does not have the expected size");

/// Negates vector \a v
inline Vector2 operator-(const Vector2& v) noexcept
{
    return Vector2(-v.x, -v.y);
}

/// Adds vector \a v2 to vector \a v1
inline Vector2 operator+(const Vector2& v1, const Vector2& v2) noexcept
{
    return Vector2(v1.x + v2.x, v1.y + v2.y);
}

/// Subtracts vector \a v2 from vector \a v1
inline Vector2 operator-(const Vector2& v1, const Vector2& v2) noexcept
{
    return Vector2(v1.x - v2.x, v1.y - v2.y);
}

/// Scales vector \a v with scalar \a s
inline Vector2 operator*(const Vector2& v, float s) noexcept
{
    return Vector2(v.x * s, v.y * s);
}

/// Scales vector \a v with scalar \a s
inline Vector2 operator*(float s, const Vector2& v) noexcept
{
    return Vector2(v.x * s, v.y * s);
}

/// Scales vector \a v with scalar 1/\a s
inline Vector2 operator/(const Vector2& v, float s) noexcept
{
    return Vector2(v.x / s, v.y / s);
}

/// Multiplies vector \a v1 with vector \a v2, component-wise
inline Vector2 operator*(const Vector2& v1, const Vector2& v2) noexcept
{
    return Vector2(v1.x * v2.x, v1.y * v2.y);
}

/// Calculates the distance between the points identified by vector \a v1 and vector \a v2
inline float distance(const Vector2& v1, const Vector2& v2) noexcept
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
inline float distance_sq(const Vector2& v1, const Vector2& v2) noexcept
{
    return v1.distance_sq(v2);
}

/// Calculates the dot product between vector \a v1 and vector \a v2
inline float dot(const Vector2& v1, const Vector2& v2) noexcept
{
    return v1.dot(v2);
}

/// Calculates the cross product between vector \a v1 and vector \a v2
inline float cross(const Vector2& v1, const Vector2& v2) noexcept
{
    return v1.cross(v2);
}

/// Normalizes vector \a v
inline Vector2 normalize(const Vector2& v) noexcept
{
    float invL = 1.0F / v.length();
    return Vector2(v.x * invL, v.y * invL);
}

/**
 * \brief Clamps each component of a vector between two extremes
 *
 * Returns \a min if \a val.{x,y} < \a min.
 * Returns \a max if \a val.{x,y} > \a max.
 * Otherwise, returns \a val.{x,y}.
 *
 * \param[in] val the vector to clamp
 * \param[in] min the lower boundary to clamp against
 * \param[in] max the upper boundary to clamp against
 *
 * \return \a val, with each component clamped between \a min and \a max.
 */
constexpr Vector2 clamp(const Vector2& val, Vector2::ComponentType min,
                        Vector2::ComponentType max) noexcept
{
    return {clamp(val.x, min, max), clamp(val.y, min, max)};
}

} // namespace khepri
