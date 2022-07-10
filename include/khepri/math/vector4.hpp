#pragma once

#include "math.hpp"
#include "vector3.hpp"

#include <gsl/gsl-lite.hpp>

#include <cassert>
#include <cmath>

namespace khepri {

class matrix;

/// \brief A 4-component vector
#pragma pack(push, 1)
class Vector4 final
{
public:
    /// The type of the vector's components
    using ComponentType = float;

    ComponentType x{}; ///< The vector's X element
    ComponentType y{}; ///< The vector's Y element
    ComponentType z{}; ///< The vector's Z element
    ComponentType w{}; ///< The vector's W element

    /// Constructs an uninitialized vector
    constexpr Vector4() noexcept = default;

    /// Constructs the vector from literal floats
    constexpr Vector4(ComponentType fx, ComponentType fy, ComponentType fz,
                      ComponentType fw) noexcept
        : x(fx), y(fy), z(fz), w(fw)
    {}

    /// Constructs the vector from a vector2, and floats for Z and W
    constexpr Vector4(const Vector2& v, ComponentType fz, ComponentType fw) noexcept
        : x(v.x), y(v.y), z(fz), w(fw)
    {}

    /// Constructs the vector from a vector3, and a float for W
    constexpr Vector4(const Vector3& v, ComponentType fw) noexcept : x(v.x), y(v.y), z(v.z), w(fw)
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
    constexpr Vector4& operator*=(ComponentType s) noexcept
    {
        x *= s;
        y *= s;
        z *= s;
        w *= s;
        return *this;
    }

    /// Scales the vector by scalar 1.0 / \a s
    constexpr Vector4& operator/=(ComponentType s) noexcept
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
    constexpr const ComponentType& operator[](std::size_t index) const noexcept
    {
        assert(index < 4);
        return gsl::span<const ComponentType>(&x, 4)[index];
    }

    /**
     * Indexes the vector.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y, etc.
     */
    constexpr ComponentType& operator[](std::size_t index) noexcept
    {
        assert(index < 4);
        return gsl::span<ComponentType>(&x, 4)[index];
    }

    /// Calculates the length of the vector
    [[nodiscard]] ComponentType length() const noexcept
    {
        return std::sqrt(length_sq());
    }

    /**
     * \brief Calculates the squared length of the vector
     *
     * Calculating the squared length (length*length) is a considerably faster operation so use it
     * whenever possible (e.g., when comparing lengths)
     */
    [[nodiscard]] constexpr ComponentType length_sq() const noexcept
    {
        return x * x + y * y + z * z + w * w;
    }

    /// Calculates the distance between the vector and vector \a v
    [[nodiscard]] ComponentType distance(const Vector4& v) const noexcept
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
    [[nodiscard]] constexpr ComponentType distance_sq(const Vector4& v) const noexcept
    {
        const auto dx = v.x - x;
        const auto dy = v.y - y;
        const auto dz = v.z - z;
        const auto dw = v.w - w;
        return dx * dx + dy * dy + dz * dz + dw * dw;
    }

    /// Calculates the dot product between the vector and vector \a v
    [[nodiscard]] constexpr ComponentType dot(const Vector4& v) const noexcept
    {
        return x * v.x + y * v.y + z * v.z + w * v.w;
    }

    /// Normalizes the vector
    void normalize() noexcept
    {
        const ComponentType inv_length = 1.0F / length();
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
static_assert(sizeof(Vector4) == 4 * sizeof(Vector4::ComponentType),
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
inline float distance(const Vector4& v1, const Vector4& v2) noexcept
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

inline constexpr Vector2::Vector2(const Vector4& v) noexcept : x(v.x), y(v.y) {}
inline constexpr Vector3::Vector3(const Vector4& v) noexcept : x(v.x), y(v.y), z(v.z) {}

/**
 * \brief Clamps each component of a vector between two extremes
 *
 * Returns \a min if \a val.{x,y,z,w} < \a min.
 * Returns \a max if \a val.{x,y,z,w} > \a max.
 * Otherwise, returns \a val.{x,y,z,w}.
 *
 * \param[in] val the vector to clamp
 * \param[in] min the lower boundary to clamp against
 * \param[in] max the upper boundary to clamp against
 *
 * \return \a val, with each component clamped between \a min and \a max.
 */
constexpr Vector4 clamp(const Vector4& val, Vector4::ComponentType min,
                        Vector4::ComponentType max) noexcept
{
    return {clamp(val.x, min, max), clamp(val.y, min, max), clamp(val.z, min, max),
            clamp(val.w, min, max)};
}

} // namespace khepri
