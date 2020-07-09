#pragma once

#include <khepri/math/vector3.hpp>

#include <cstdint>
#include <vector>

namespace khepri::renderer {

/**
 * \brief A mesh
 *
 * A mesh consists of consecutive sets of triangle-faces.
 * The triangles are considered front-facing with CCW.
 */
struct Mesh final
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