#pragma once

#include "material.hpp"
#include "material_id.hpp"
#include "mesh_id.hpp"
#include "texture_id.hpp"

#include <khepri/math/matrix.hpp>
#include <khepri/math/vector2.hpp>
#include <khepri/math/vector3.hpp>
#include <khepri/math/vector4.hpp>

#include <gsl/gsl-lite.hpp>

#include <cstdint>
#include <string>
#include <variant>

namespace khepri::renderer {

/**
 * \brief An instance of a mesh
 *
 * A mesh is just a template for the renderer. It can be instantiated multiple times with
 * this type. A #khepri::renderer::Renderer renders these instances.
 *
 * \see #khepri::renderer::Renderer::render_meshes
 */
struct MeshInstance
{
    /// Parameter value
    using ParamValue = Material::PropertyValue;

    /// Parameter description
    struct Param
    {
        /// Name of the parameter
        std::string name;
        /// Value of the parameter
        ParamValue value;
    };

    /// Identifies the mesh this is an instance of
    MeshId mesh_id;

    /// The transformation matrix for this instance
    Matrix transform;

    /// The ID of the material to render this instance with
    MaterialId material_id;

    /// Material parameters for this instance
    gsl::span<const Param> material_params;
};

} // namespace khepri::renderer
