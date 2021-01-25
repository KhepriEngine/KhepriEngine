#pragma once

#include <cstddef>

namespace khepri::renderer {

/**
 * \brief Identifier for a material.
 *
 * A #material is a collection of shaders and a collection of related properties for rendering a
 * #Mesh. This type is used to identify materials. Materials are created and destroyed by a
 * #renderer.
 *
 * \see #Renderer::create_material
 */
using MaterialId = std::size_t;

} // namespace khepri::renderer
