#pragma once

#include <cstddef>

namespace khepri::renderer {

/**
 * \brief Identifier for a texture.
 *
 * A texture can be specified as material parameter and used in shaders to render meshes. This type
 * is used to identify textures. Textures are created and destroyed by a #renderer.
 *
 * \see #Renderer::create_texture
 */
using TextureId = std::size_t;

} // namespace khepri::renderer
