#pragma once

#include <cstddef>

namespace khepri::renderer {

/**
 * \brief A material.
 *
 * A material is a collection of shaders and a collection of related properties for rendering a
 * #khepri::renderer::Mesh. Materials are created by a #khepri::renderer::Renderer.
 *
 * \see #khepri::renderer::Renderer::create_material
 */
class Material
{
public:
    Material()          = default;
    virtual ~Material() = default;

    Material(const Material&) = delete;
    Material(Material&&)      = delete;
    Material& operator=(const Material&) = delete;
    Material& operator=(Material&&) = delete;
};

} // namespace khepri::renderer
