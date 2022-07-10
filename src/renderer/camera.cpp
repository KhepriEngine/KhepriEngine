#include <khepri/math/math.hpp>
#include <khepri/renderer/camera.hpp>

#include <algorithm>
#include <cassert>

namespace khepri::renderer {

Camera::Matrices Camera::create_matrices(const Properties& properties) noexcept
{
    Matrices matrices;
    matrices.view =
        Matrix::create_look_at_view(properties.position, properties.target, properties.up);
    matrices.projection =
        (properties.type == Type::orthographic)
            ? Matrix::create_orthographic_projection(properties.width, properties.aspect,
                                                     properties.znear, properties.zfar)
            : Matrix::create_perspective_projection(properties.fov, properties.aspect,
                                                    properties.znear, properties.zfar);
    matrices.view_proj     = matrices.view * matrices.projection;
    matrices.view_inv      = inverse(matrices.view);
    matrices.view_proj_inv = inverse(matrices.view_proj);
    return matrices;
}

Camera::Camera(const Properties& properties)
    : m_properties(properties)
    , m_matrices(create_matrices(properties))
    , m_frustum(frustum({-1, -1}, {1, 1}))
{}

Frustum Camera::frustum(const Vector2& p1, const Vector2& p2) const noexcept
{
    // Constructs a side plane from its coordinates on the near plane (-1 <= x,y <= 1)
    // The @orthogonal_view_dir lies in the plane, orthogonal to the view direction.
    const auto create_side_plane = [&](float x, float y, const Vector3& orthogonal_view_dir) {
        Vector3 near_position = m_matrices.view_proj_inv.transform_coord({x, y, 0.0f});
        Vector3 far_position  = m_matrices.view_proj_inv.transform_coord({x, y, 1.0f});
        Vector3 inside_dir    = normalize(cross(far_position - near_position, orthogonal_view_dir));
        return Plane(near_position, inside_dir);
    };

    // Calculate world-space directions of camera-space view, right and up.
    const Vector3 view_dir  = normalize(m_properties.target - m_properties.position);
    const Vector3 right_dir = normalize(cross(view_dir, m_properties.up));
    const Vector3 up_dir    = normalize(cross(right_dir, view_dir));

    // Get the bounds
    const auto [min_x, max_x] = std::minmax(p1.x, p2.x);
    const auto [min_y, max_y] = std::minmax(p1.y, p2.y);

    // Construct the view frustum
    const Plane left   = create_side_plane(min_x, min_y, up_dir);
    const Plane right  = create_side_plane(max_x, min_y, -up_dir);
    const Plane top    = create_side_plane(min_x, max_y, right_dir);
    const Plane bottom = create_side_plane(min_x, min_y, -right_dir);
    const Plane near(m_properties.position + m_properties.znear * view_dir, view_dir);
    const Plane far(m_properties.position + m_properties.zfar * view_dir, -view_dir);

    return Frustum(left, right, top, bottom, near, far);
}

std::tuple<Vector3, Vector3> Camera::unproject(const Vector2& coords) const noexcept
{
    return {
        m_matrices.view_proj_inv.transform_coord({coords, 0.0F}), // Near
        m_matrices.view_proj_inv.transform_coord({coords, 1.0F})  // far
    };
}

float Camera::lod(const Vector3& world_pos) const noexcept
{
    // The LOD is just the Z-position in the view frustum
    // (but inverted: 1 is near plane, 0 is far plane)
    const auto v = Vector4(world_pos, 1) * m_matrices.view_proj;
    return saturate((m_properties.zfar - v.w) / (m_properties.zfar - m_properties.znear));
}

void Camera::properties(const Properties& properties) noexcept
{
    // The view direction and the "up" vector may not be colinear;
    // otherwise we have a degree of freedom in the camera transformation and
    // the matrix calculations will be messed up.
    assert(!colinear(properties.up, properties.target - properties.position));

    m_properties = properties;
    m_matrices   = create_matrices(properties);
    m_frustum    = frustum({-1, -1}, {1, 1});
}

} // namespace khepri::renderer
