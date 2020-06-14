#include <khepri/renderer/model.hpp>

#include <algorithm>
#include <cassert>

namespace khepri::renderer {

static Sphere compute_bounding_sphere(const std::vector<Model::Mesh>& meshes)
{
    // We center the bounding sphere at 0,0,0 in object space because that's
    // the pivot point for rotations. That means we don't need to consider the
    // rotation of the object when applying its bounding sphere.
    const Vector3 center(0, 0, 0);

    // Degenerate sphere by default
    float radius = 0.0F;

    if (!meshes.empty()) {
        for (const auto& mesh : meshes) {
            for (const auto& v : mesh.vertices) {
                radius = std::max(radius, (v.position - center).length());
            }
        }
    }
    return Sphere(center, radius);
}

Model::CollisionMesh::CollisionMesh(std::vector<Vector3> vertices, std::vector<Index> indices)
    : m_vertices(std::move(vertices)), m_indices(std::move(indices))
{
    assert(m_indices.size() % 3 == 0);
}

// Möller-Trumbore ray/triangle intersection algorithm
static float intersect_distance(const Ray& ray, const Vector3& v0, const Vector3& v1,
                                const Vector3& v2)
{
    Vector3 e1 = v1 - v0;
    Vector3 e2 = v2 - v0;

    // Calculate determinant
    Vector3 h   = cross(ray.direction(), e2);
    float   det = dot(e1, h);

    constexpr auto MAX_PARALLEL_DETERMINANT = 0.00001;
    if (det < MAX_PARALLEL_DETERMINANT) {
        // If the determinant is negative the triangle is backfacing.
        // If the determinant is close to 0, the ray lies on or parallel to triangle.
        return -1;
    }
    float inv_det = 1.0F / det;

    // First barycentric coordinate (u) of intersection point
    Vector3 s = ray.start() - v0;
    float   u = inv_det * dot(s, h);
    if (u < 0.0F || u > 1.0F) {
        // Intersection is outside the triangle.
        return -1;
    }

    // Second barycentric coordinate (v) of intersection point
    Vector3 q = cross(s, e1);
    float   v = inv_det * dot(ray.direction(), q);
    if (v < 0.0F || u + v > 1.0F) {
        // Intersection is outside the triangle.
        return -1;
    }

    // Get the distance along the ray of the intersection point
    float d = inv_det * dot(e2, q);
    if (d < 0.0F) {
        // Intersection lies behind ray starting point
        return -1;
    }

    return d;
}

float Model::CollisionMesh::intersect_distance(const Ray& ray) const
{
    // Unoptimized implementation: iterate through the triangles and find the closest one that
    // intersects
    float min_distance = std::numeric_limits<float>::max();
    bool  found        = false;

    for (std::size_t i = 0; i < m_indices.size(); i += 3) {
        float distance = renderer::intersect_distance(ray, m_vertices[m_indices[i + 0]],
                                                      m_vertices[m_indices[i + 1]],
                                                      m_vertices[m_indices[i + 2]]);

        if (distance != -1) {
            min_distance = std::min(min_distance, distance);
            found        = true;
        }
    }

    return found ? min_distance : -1.0F;
}

bool Model::CollisionMesh::intersect(const Frustum& frustum) const
{
    // Every vertex has to be inside the frustum
    return std::all_of(m_vertices.begin(), m_vertices.end(),
                       [&](const auto& v) { return frustum.inside(v); });
}

static Model::CollisionMesh create_collision_mesh(const std::vector<Model::Mesh>& meshes)
{
    std::vector<Vector3>      vertices;
    std::vector<Model::Index> indices;

    if (!meshes.empty()) {
        vertices.reserve(meshes[0].vertices.size());
        for (const auto& v : meshes[0].vertices) {
            vertices.push_back(v.position);
        }
        indices = meshes[0].indices;
    }

    return Model::CollisionMesh(std::move(vertices), std::move(indices));
}

Model::Model(std::vector<Mesh> meshes)
    : m_meshes(std::move(meshes))
    , m_bounding_sphere(compute_bounding_sphere(m_meshes))
    , m_collision_mesh(create_collision_mesh(m_meshes))
{}

} // namespace khepri::renderer
