#include <khepri/math/matrix.hpp>
#include <khepri/math/quaternion.hpp>

#include <cassert>

namespace khepri {

const Matrix Matrix::IDENTITY(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

Matrix operator*(const Matrix& m1, const Matrix& m2) noexcept
{
    return Matrix(
        m1(0, 0) * m2(0, 0) + m1(0, 1) * m2(1, 0) + m1(0, 2) * m2(2, 0) + m1(0, 3) * m2(3, 0),
        m1(0, 0) * m2(0, 1) + m1(0, 1) * m2(1, 1) + m1(0, 2) * m2(2, 1) + m1(0, 3) * m2(3, 1),
        m1(0, 0) * m2(0, 2) + m1(0, 1) * m2(1, 2) + m1(0, 2) * m2(2, 2) + m1(0, 3) * m2(3, 2),
        m1(0, 0) * m2(0, 3) + m1(0, 1) * m2(1, 3) + m1(0, 2) * m2(2, 3) + m1(0, 3) * m2(3, 3),

        m1(1, 0) * m2(0, 0) + m1(1, 1) * m2(1, 0) + m1(1, 2) * m2(2, 0) + m1(1, 3) * m2(3, 0),
        m1(1, 0) * m2(0, 1) + m1(1, 1) * m2(1, 1) + m1(1, 2) * m2(2, 1) + m1(1, 3) * m2(3, 1),
        m1(1, 0) * m2(0, 2) + m1(1, 1) * m2(1, 2) + m1(1, 2) * m2(2, 2) + m1(1, 3) * m2(3, 2),
        m1(1, 0) * m2(0, 3) + m1(1, 1) * m2(1, 3) + m1(1, 2) * m2(2, 3) + m1(1, 3) * m2(3, 3),

        m1(2, 0) * m2(0, 0) + m1(2, 1) * m2(1, 0) + m1(2, 2) * m2(2, 0) + m1(2, 3) * m2(3, 0),
        m1(2, 0) * m2(0, 1) + m1(2, 1) * m2(1, 1) + m1(2, 2) * m2(2, 1) + m1(2, 3) * m2(3, 1),
        m1(2, 0) * m2(0, 2) + m1(2, 1) * m2(1, 2) + m1(2, 2) * m2(2, 2) + m1(2, 3) * m2(3, 2),
        m1(2, 0) * m2(0, 3) + m1(2, 1) * m2(1, 3) + m1(2, 2) * m2(2, 3) + m1(2, 3) * m2(3, 3),

        m1(3, 0) * m2(0, 0) + m1(3, 1) * m2(1, 0) + m1(3, 2) * m2(2, 0) + m1(3, 3) * m2(3, 0),
        m1(3, 0) * m2(0, 1) + m1(3, 1) * m2(1, 1) + m1(3, 2) * m2(2, 1) + m1(3, 3) * m2(3, 1),
        m1(3, 0) * m2(0, 2) + m1(3, 1) * m2(1, 2) + m1(3, 2) * m2(2, 2) + m1(3, 3) * m2(3, 2),
        m1(3, 0) * m2(0, 3) + m1(3, 1) * m2(1, 3) + m1(3, 2) * m2(2, 3) + m1(3, 3) * m2(3, 3));
}

Matrix operator*(const Matrix& m, float s) noexcept
{
    return Matrix(m(0, 0) * s, m(0, 1) * s, m(0, 2) * s, m(0, 3) * s, m(1, 0) * s, m(1, 1) * s,
                  m(1, 2) * s, m(1, 3) * s, m(2, 0) * s, m(2, 1) * s, m(2, 2) * s, m(2, 3) * s,
                  m(3, 0) * s, m(3, 1) * s, m(3, 2) * s, m(3, 3) * s);
}

Matrix& Matrix::operator*=(float s) noexcept
{
    Matrix& m = *this;
    m(0, 0) *= s, m(0, 1) *= s, m(0, 2) *= s, m(0, 3) *= s, m(1, 0) *= s, m(1, 1) *= s,
        m(1, 2) *= s, m(1, 3) *= s, m(2, 0) *= s, m(2, 1) *= s, m(2, 2) *= s, m(2, 3) *= s,
        m(3, 0) *= s, m(3, 1) *= s, m(3, 2) *= s, m(3, 3) *= s;
    return *this;
}

Vector4 operator*(const Vector4& v, const Matrix& m) noexcept
{
    return Vector4(v.x * m(0, 0) + v.y * m(1, 0) + v.z * m(2, 0) + v.w * m(3, 0),
                   v.x * m(0, 1) + v.y * m(1, 1) + v.z * m(2, 1) + v.w * m(3, 1),
                   v.x * m(0, 2) + v.y * m(1, 2) + v.z * m(2, 2) + v.w * m(3, 2),
                   v.x * m(0, 3) + v.y * m(1, 3) + v.z * m(2, 3) + v.w * m(3, 3));
}

Vector3 operator*(const Vector3& v, const Matrix& m) noexcept
{
    return Vector3(v.x * m(0, 0) + v.y * m(1, 0) + v.z * m(2, 0),
                   v.x * m(0, 1) + v.y * m(1, 1) + v.z * m(2, 1),
                   v.x * m(0, 2) + v.y * m(1, 2) + v.z * m(2, 2));
}

Vector3 Matrix::transform_coord(const Vector3& v) const noexcept
{
    const Vector4 a = Vector4(v, 1.0F) * *this;
    return Vector3(a.x / a.w, a.y / a.w, a.z / a.w);
}

Matrix Matrix::create_rotation(const Quaternion& q) noexcept
{
    return Matrix(1 - 2 * q.y * q.y - 2 * q.z * q.z, 2 * q.x * q.y + 2 * q.w * q.z,
                  2 * q.x * q.z - 2 * q.w * q.y, 0, 2 * q.x * q.y - 2 * q.w * q.z,
                  1 - 2 * q.x * q.x - 2 * q.z * q.z, 2 * q.y * q.z + 2 * q.w * q.x, 0,
                  2 * q.x * q.z + 2 * q.w * q.y, 2 * q.y * q.z - 2 * q.w * q.x,
                  1 - 2 * q.x * q.x - 2 * q.y * q.y, 0, 0, 0, 0, 1);
}

Matrix Matrix::create_srt(const Vector3& s, const Quaternion& r, const Vector3& t) noexcept
{
    assert(s.x > 0.0F);
    assert(s.y > 0.0F);
    assert(s.z > 0.0F);

    // Creates a matrix equal to M_s * M_r * M_t
    // This is just similar to matrix(const quaternion &q), but each row scaled with s
    // The last row is just the translation
    return Matrix(s.x * (1 - 2 * r.y * r.y - 2 * r.z * r.z), s.x * (2 * r.x * r.y + 2 * r.w * r.z),
                  s.x * (2 * r.x * r.z - 2 * r.w * r.y), 0, s.y * (2 * r.x * r.y - 2 * r.w * r.z),
                  s.y * (1 - 2 * r.x * r.x - 2 * r.z * r.z), s.y * (2 * r.y * r.z + 2 * r.w * r.x),
                  0, s.z * (2 * r.x * r.z + 2 * r.w * r.y), s.z * (2 * r.y * r.z - 2 * r.w * r.x),
                  s.z * (1 - 2 * r.x * r.x - 2 * r.y * r.y), 0, t.x, t.y, t.z, 1);
}

void Matrix::transpose() noexcept
{
    for (int i = 1; i < 4; i++) {
        for (int j = 0; j < i; j++) {
            float tmp     = (*this)(i, j);
            (*this)(i, j) = (*this)(j, i);
            (*this)(j, i) = tmp;
        }
    }
}

Matrix transpose(const Matrix& m) noexcept
{
    Matrix x;
    for (int i = 1; i < 4; i++) {
        for (int j = 0; j < i; j++) {
            x(i, j) = m(j, i);
            x(j, i) = m(i, j);
        }
    }
    return x;
}

Matrix Matrix::inverse() const noexcept
{
    Matrix        dst;
    const Matrix& src = *this;

    dst(0, 0) = src(1, 1) * (src(2, 2) * src(3, 3) - src(2, 3) * src(3, 2)) -
                src(2, 1) * (src(1, 2) * src(3, 3) - src(1, 3) * src(3, 2)) +
                src(3, 1) * (src(1, 2) * src(2, 3) - src(1, 3) * src(2, 2));

    dst(1, 0) = src(2, 0) * (src(1, 2) * src(3, 3) - src(1, 3) * src(3, 2)) -
                src(1, 0) * (src(2, 2) * src(3, 3) - src(2, 3) * src(3, 2)) -
                src(3, 0) * (src(1, 2) * src(2, 3) - src(1, 3) * src(2, 2));

    dst(2, 0) = src(1, 0) * (src(2, 1) * src(3, 3) - src(2, 3) * src(3, 1)) -
                src(2, 0) * (src(1, 1) * src(3, 3) - src(1, 3) * src(3, 1)) +
                src(3, 0) * (src(1, 1) * src(2, 3) - src(1, 3) * src(2, 1));

    dst(3, 0) = src(2, 0) * (src(1, 1) * src(3, 2) - src(1, 2) * src(3, 1)) -
                src(1, 0) * (src(2, 1) * src(3, 2) - src(2, 2) * src(3, 1)) -
                src(3, 0) * (src(1, 1) * src(2, 2) - src(1, 2) * src(2, 1));

    dst(0, 1) = src(2, 1) * (src(0, 2) * src(3, 3) - src(0, 3) * src(3, 2)) -
                src(0, 1) * (src(2, 2) * src(3, 3) - src(2, 3) * src(3, 2)) -
                src(3, 1) * (src(0, 2) * src(2, 3) - src(0, 3) * src(2, 2));

    dst(1, 1) = src(0, 0) * (src(2, 2) * src(3, 3) - src(2, 3) * src(3, 2)) -
                src(2, 0) * (src(0, 2) * src(3, 3) - src(0, 3) * src(3, 2)) +
                src(3, 0) * (src(0, 2) * src(2, 3) - src(0, 3) * src(2, 2));

    dst(2, 1) = src(2, 0) * (src(0, 1) * src(3, 3) - src(0, 3) * src(3, 1)) -
                src(0, 0) * (src(2, 1) * src(3, 3) - src(2, 3) * src(3, 1)) -
                src(3, 0) * (src(0, 1) * src(2, 3) - src(0, 3) * src(2, 1));

    dst(3, 1) = src(0, 0) * (src(2, 1) * src(3, 2) - src(2, 2) * src(3, 1)) -
                src(2, 0) * (src(0, 1) * src(3, 2) - src(0, 2) * src(3, 1)) +
                src(3, 0) * (src(0, 1) * src(2, 2) - src(0, 2) * src(2, 1));

    dst(0, 2) = src(0, 1) * (src(1, 2) * src(3, 3) - src(1, 3) * src(3, 2)) -
                src(1, 1) * (src(0, 2) * src(3, 3) - src(0, 3) * src(3, 2)) +
                src(3, 1) * (src(0, 2) * src(1, 3) - src(0, 3) * src(1, 2));

    dst(1, 2) = src(1, 0) * (src(0, 2) * src(3, 3) - src(0, 3) * src(3, 2)) -
                src(0, 0) * (src(1, 2) * src(3, 3) - src(1, 3) * src(3, 2)) -
                src(3, 0) * (src(0, 2) * src(1, 3) - src(0, 3) * src(1, 2));

    dst(2, 2) = src(0, 0) * (src(1, 1) * src(3, 3) - src(1, 3) * src(3, 1)) -
                src(1, 0) * (src(0, 1) * src(3, 3) - src(0, 3) * src(3, 1)) +
                src(3, 0) * (src(0, 1) * src(1, 3) - src(0, 3) * src(1, 1));

    dst(3, 2) = src(1, 0) * (src(0, 1) * src(3, 2) - src(0, 2) * src(3, 1)) -
                src(0, 0) * (src(1, 1) * src(3, 2) - src(1, 2) * src(3, 1)) -
                src(3, 0) * (src(0, 1) * src(1, 2) - src(0, 2) * src(1, 1));

    dst(0, 3) = src(1, 1) * (src(0, 2) * src(2, 3) - src(0, 3) * src(2, 2)) -
                src(0, 1) * (src(1, 2) * src(2, 3) - src(1, 3) * src(2, 2)) -
                src(2, 1) * (src(0, 2) * src(1, 3) - src(0, 3) * src(1, 2));

    dst(1, 3) = src(0, 0) * (src(1, 2) * src(2, 3) - src(1, 3) * src(2, 2)) -
                src(1, 0) * (src(0, 2) * src(2, 3) - src(0, 3) * src(2, 2)) +
                src(2, 0) * (src(0, 2) * src(1, 3) - src(0, 3) * src(1, 2));

    dst(2, 3) = src(1, 0) * (src(0, 1) * src(2, 3) - src(0, 3) * src(2, 1)) -
                src(0, 0) * (src(1, 1) * src(2, 3) - src(1, 3) * src(2, 1)) -
                src(2, 0) * (src(0, 1) * src(1, 3) - src(0, 3) * src(1, 1));

    dst(3, 3) = src(0, 0) * (src(1, 1) * src(2, 2) - src(1, 2) * src(2, 1)) -
                src(1, 0) * (src(0, 1) * src(2, 2) - src(0, 2) * src(2, 1)) +
                src(2, 0) * (src(0, 1) * src(1, 2) - src(0, 2) * src(1, 1));

    // Calculate determinant.
    // If zero, the matrix is uninvertible.
    const float det = (src(0, 0) * dst(0, 0) + src(0, 1) * dst(1, 0) + src(0, 2) * dst(2, 0) +
                       src(0, 3) * dst(3, 0));
    assert(det != 0);

    // Divide matrix by determinant
    const float invdet = 1 / det;
    return dst * invdet;
}

// Constructs a right-handed LookAt view matrix
Matrix Matrix::create_look_at_view(const Vector3& eye, const Vector3& at,
                                   const Vector3& up) noexcept
{
    const Vector3 zaxis = normalize(eye - at);
    const Vector3 xaxis = normalize(cross(up, zaxis));
    const Vector3 yaxis = cross(zaxis, xaxis);

    return Matrix(xaxis.x, yaxis.x, zaxis.x, 0, xaxis.y, yaxis.y, zaxis.y, 0, xaxis.z, yaxis.z,
                  zaxis.z, 0, -dot(xaxis, eye), -dot(yaxis, eye), -dot(zaxis, eye), 1);
}

// Constructs a right-handed perspective projection matrix
Matrix Matrix::create_perspective_projection(float fovy, float aspect, float zn, float zf) noexcept
{
    const float y_scale = 1 / std::tan(fovy / 2);
    const float x_scale = y_scale / aspect;

    return Matrix(x_scale, 0, 0, 0, 0, y_scale, 0, 0, 0, 0, zf / (zn - zf), -1, 0, 0,
                  zn * zf / (zn - zf), 0);
}

// Constructs a right-handed orthographic projection matrix
Matrix Matrix::create_orthographic_projection(float w, float aspect, float zn, float zf) noexcept
{
    float h = w / aspect;
    return Matrix(2 / w, 0, 0, 0, 0, 2 / h, 0, 0, 0, 0, 1 / (zn - zf), 0, 0, 0, zn / (zn - zf), 1);
}

} // namespace khepri