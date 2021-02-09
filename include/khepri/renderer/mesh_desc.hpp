#pragma once

#include <khepri/math/vector2.hpp>
#include <khepri/math/vector3.hpp>

#include <cstdint>
#include <vector>

namespace khepri::renderer {

/**
 * \brief Description of a mesh
 *
 * A mesh consists of consecutive sets of triangle-faces.
 */
struct MeshDesc final
{
    /// Integer size of indexes
    using Index = std::uint16_t;

    /// Describes a single vertex in the mesh
    struct Vertex
    {
        /// The vertex' position
        Vector3 position;

        /// The vertex' normal vector
        Vector3 normal;

        /// The vertex' tangent vector
        Vector3 tangent;

        /// The vertex' binormal vector
        Vector3 binormal;

        /// The vertex' texture coordinate
        Vector2 uv;
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

} // namespace khepri::renderer
