#pragma once

#include <khepri/math/frustum.hpp>
#include <khepri/math/ray.hpp>
#include <khepri/math/sphere.hpp>
#include <khepri/math/vector2.hpp>
#include <khepri/math/vector3.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace khepri::renderer {

/**
 * \brief A renderer-independent model definition
 */
class Model
{
public:
    /// Integer size of indexes
    using Index = std::uint16_t;

    /**
     * \brief A mesh
     *
     * A mesh consists of consecutive triangles in the m_indices vector.
     * The triangles are considered front-facing with CCW.
     */
    struct Mesh final
    {
        /// Describes a single vertex in the mesh
        struct Vertex
        {
            /// The vertex' position
            Vector3 position;

            /// The vertex' normal vector
            Vector3 normal;
        };

        /// The vertices in the mesh
        std::vector<Vertex> vertices;

        /**
         * \brief The indices of the faces in the mesh.
         *
         * Each face is defined by a consecutive triplet of indices into #vertices.
         */
        std::vector<Index> indices;
    };

    /**
     * \brief Collision mesh
     *
     * A collision mesh is an optimized data structure for collision detection.
     */
    class CollisionMesh final
    {
    public:
        /// Constructs a colltion mesh from vertices and faces
        CollisionMesh(std::vector<Vector3> vertices, std::vector<Index> indices);

        /**
         * \brief Returns the distance along the ray of the first intersection with this mesh, if
         * any.
         *
         * Returns a negative number if there is no intersection or if the origin is inside the
         * mesh.
         */
        [[nodiscard]] float intersect_distance(const Ray& ray) const;

        /// Checks if this mesh intersects, even partially, the specified frustum.
        [[nodiscard]] bool intersect(const Frustum& frustum) const;

    private:
        std::vector<Vector3> m_vertices;
        std::vector<Index>   m_indices;
    };

    /// Constructs a model from meshes
    Model(std::vector<Mesh> meshes);

    /// Returns the meshes in this model
    [[nodiscard]] const std::vector<Mesh>& meshes() const noexcept
    {
        return m_meshes;
    }

    /// Returns the bounding sphere of this model
    [[nodiscard]] const Sphere& bounding_sphere() const noexcept
    {
        return m_bounding_sphere;
    }

    /// Returns the collision mesh of this model
    CollisionMesh& collision_mesh()
    {
        return m_collision_mesh;
    }

private:
    std::vector<Mesh>   m_meshes;
    Sphere              m_bounding_sphere;
    class CollisionMesh m_collision_mesh;
};

} // namespace khepri::renderer
