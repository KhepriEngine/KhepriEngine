#pragma once

#include "vector3.hpp"
#include "vector4.hpp"

#include <array>

namespace khepri {

class Quaternion;

/**
 * \brief 4x4 matrix
 *
 * \note the matrix is stored in column-major order.
 */
class Matrix final
{
    friend Matrix operator*(const Matrix& m1, const Matrix& m2) noexcept;

    std::array<Vector4, 4> m_cols{};

public:
    /// The type of the matrix's components
    using ComponentType = Vector4::ComponentType;

    /// Constructs an uninitialized matrix
    Matrix() noexcept = default;

    /// Constructs the matrix from 16 floats, row-major order
    Matrix(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24,
           float m31, float m32, float m33, float m34, float m41, float m42, float m43,
           float m44) noexcept
    {
        m_cols[0] = Vector4(m11, m21, m31, m41);
        m_cols[1] = Vector4(m12, m22, m32, m42);
        m_cols[2] = Vector4(m13, m23, m33, m43);
        m_cols[3] = Vector4(m14, m24, m34, m44);
    }

    /// Constructs the matrix from 4 column vectors
    Matrix(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4) noexcept
    {
        m_cols[0] = v1;
        m_cols[1] = v2;
        m_cols[2] = v3;
        m_cols[3] = v4;
    }

    /// Post-multiplies the matrix with \a mat
    Matrix& operator*=(const Matrix& mat) noexcept
    {
        return *this = *this * mat;
    }

    /// Scales all elements of the matrix
    Matrix& operator*=(float s) noexcept;

    /// Scales all elements of the matrix
    Matrix& operator/=(float s) noexcept
    {
        return *this *= (1.0F / s);
    }

    /// Returns an element of the matrix
    constexpr const float& operator()(std::size_t row, std::size_t col) const
    {
        return m_cols.at(col)[row];
    }

    /// Returns an element of the matrix
    constexpr float& operator()(std::size_t row, std::size_t col)
    {
        return m_cols.at(col)[row];
    }

    /// Returns a column of the matrix
    constexpr const Vector4& operator()(std::size_t col) const noexcept
    {
        return m_cols.at(col);
    }

    /// Returns a column of the matrix
    constexpr Vector4& operator()(std::size_t col) noexcept
    {
        return m_cols.at(col);
    }

    /// Transform (post-multiply) \a v with this matrix, assuming 1.0 as w component
    [[nodiscard]] Vector3 transform_coord(const Vector3& v) const noexcept;

    /// Transposes the matrix
    void transpose() noexcept;

    /// Returns the inverse matrix.
    /// \note is undefined behavior if the matrix is not invertible.
    [[nodiscard]] Matrix inverse() const noexcept;

    /// Returns the translation part (4th row) of the matrix
    [[nodiscard]] Vector3 get_translation() const noexcept
    {
        return Vector3(m_cols[0].w, m_cols[1].w, m_cols[2].w);
    }

    /// Sets the translation components of this matrix
    void set_translation(const Vector3& v) noexcept
    {
        m_cols[0].w = v.x;
        m_cols[1].w = v.y;
        m_cols[2].w = v.z;
    }

    /// Returns the rotation and scale matrix (the top-left 3x3 submatrix)
    [[nodiscard]] Matrix get_rotation_scale() const noexcept;

    /// Constructs a matrix from Scale, Rotation and Translation
    static Matrix create_srt(const Vector3& scale, const Quaternion& rotation,
                             const Vector3& translation) noexcept;

    /// Constructs a rotation transformation from the specified quaternion
    static Matrix create_rotation(const Quaternion& q) noexcept;

    /// Constructs a scale transformation from a scale factor
    static Matrix create_scaling(float scale) noexcept
    {
        return Matrix(scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, 1);
    }

    /// Constructs a scale transformation from a scale vector
    static Matrix create_scaling(const Vector3& scale) noexcept
    {
        return Matrix(scale.x, 0, 0, 0, 0, scale.y, 0, 0, 0, 0, scale.z, 0, 0, 0, 0, 1);
    }

    /// Constructs a translation transformation from a translation vector
    static Matrix create_translation(const Vector3& translation) noexcept
    {
        return Matrix(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, translation.x, translation.y,
                      translation.z, 1);
    }

    /// Creates a camera-style transformation matrix
    static Matrix create_look_at_view(
        const Vector3& eye, ///< [in] the world-space position of the camera
        const Vector3& at,  ///< [in] the world-space target position the camera is looking at
        const Vector3& up   ///< [in] the world-space vector which will point up on the camera image
        ) noexcept;

    /// Creates a perspective projection matrix
    static Matrix create_perspective_projection(
        float fovy,   ///< [in] the vertical Field-of-View angle, in radians
        float aspect, ///< [in] the aspect ration (width / height) of the viewport
        float zn,     ///< [in] the distance from the origin to the near clip plane
        float zf      ///< [in] the distance from the origin to the far clip plane
        ) noexcept;

    /// Creates an orthographic projection matrix
    static Matrix create_orthographic_projection(
        float width,  ///< [in] width of the camera image, in world-space units
        float aspect, ///< [in] the aspect ration (width / height) of the viewport
        float zn,     ///< [in] the distance from the origin to the near clip plane
        float zf      ///< [in] the distance from the origin to the far clip plane
        ) noexcept;

    /// Identity matrix
    static const Matrix IDENTITY;
};

/// Transforms (Post-multiplies) a vector with a matrix
Vector4 operator*(const Vector4& v, const Matrix& m) noexcept;

// Transforms (Post-multiplies) a vector with a matrix.
// Same as: Vector3(Vector4(v, 0) * m);
Vector3 operator*(const Vector3& v, const Matrix& m) noexcept;

/// Post-multiplies two matrices
Matrix operator*(const Matrix& m1, const Matrix& m2) noexcept;

/// Scales all elements of the matrix
Matrix operator*(const Matrix& m, float s) noexcept;

/// Scales all elements of the matrix
inline Matrix operator*(float s, const Matrix& m) noexcept
{
    return m * s;
}

/// Scales all elements of the matrix
inline Matrix operator/(const Matrix& m, float s) noexcept
{
    return m * (1.0F / s);
}

/// Returns the inverse matrix
/// \note is undefined behavior if \a m is not invertible.
inline Matrix inverse(const Matrix& m) noexcept
{
    return m.inverse();
}

/// Transposes the matrix
Matrix transpose(const Matrix& m) noexcept;

} // namespace khepri
