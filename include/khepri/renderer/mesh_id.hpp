#pragma once

#include <cstddef>

namespace khepri::renderer {

/**
 * \brief Identifier for a mesh.
 *
 * A mesh is a collection of geometry that can be rendered. This type is used to identify meshes.
 * Meshes are created and destroyed by a #renderer.
 *
 * \see #Renderer::create_mesh
 */
using MeshId = std::size_t;

} // namespace khepri::renderer
