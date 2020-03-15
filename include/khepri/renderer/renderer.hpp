#pragma once

namespace khepri::renderer {

/**
 * \brief Interface for renderers
 *
 * This interface provides a technology-independent interface to various renderers.
 */
class Renderer
{
public:
    Renderer()          = default;
    virtual ~Renderer() = default;

    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&)      = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    /**
     * Renders.
     */
    virtual void render() = 0;
};

} // namespace khepri::renderer
