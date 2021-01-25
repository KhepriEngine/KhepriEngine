#pragma once

#include <cstddef>

namespace khepri::renderer {

/**
 * \brief Identifier for a shader.
 *
 * A shader is a program that be executed by a renderer to render a mesh. This type is used to
 * identify shaders. Shaders are created and destroyed by a #renderer.
 *
 * \see #Renderer::create_shader
 */
using ShaderId = std::size_t;

} // namespace khepri::renderer
