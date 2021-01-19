#pragma once

#include "renderable_mesh_id.hpp"

#include <khepri/math/matrix.hpp>

namespace khepri::renderer {

/**
 * \brief An instance of a renderable mesh
 *
 * A renderable mesh is just a template for the renderer. It can be instantiated many times with
 * this type. The #khepri::renderer::Renderer renders these instances.
 */
struct RenderableMeshInstance
{
    /// Identifies the mesh this is an instance of
    RenderableMeshId mesh_id{};

    /// The transformation matrix for this instance
    Matrix transform{};
};

} // namespace khepri::renderer
