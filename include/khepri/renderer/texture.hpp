#pragma once

namespace khepri::renderer {

/**
 * \brief A texture
 *
 * A texture can be specified as material parameter and used in shaders to render meshes. Textures
 * are created by a #khepri::renderer::Renderer.
 *
 * \see #khepri::renderer::Renderer::create_texture
 */
class Texture
{
public:
    Texture()          = default;
    virtual ~Texture() = default;

    Texture(const Texture&) = delete;
    Texture(Texture&&)      = delete;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&&) = delete;
};

} // namespace khepri::renderer
