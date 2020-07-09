#pragma once

#include <cstddef>

namespace khepri::renderer {

/**
 * \brief Identifier for a renderable mesh.
 *
 * A renderable mesh is a mesh that has been can be renderer. This type is used to identify such
 * meshes. Renderables meshes are created and destroyed by a #renderer.
 *
 * \see #renderer::create_renderable_mesh
 */
using renderable_mesh_id = std::size_t;

} // namespace khepri::renderer
