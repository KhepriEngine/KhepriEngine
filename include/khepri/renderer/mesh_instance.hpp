#pragma once

#include "material.hpp"
#include "material_desc.hpp"
#include "mesh.hpp"
#include "texture.hpp"

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
    using ParamValue = MaterialDesc::PropertyValue;

    /// Parameter description
    struct Param
    {
        /// Name of the parameter
        std::string name;
        /// Value of the parameter
        ParamValue value;
    };

    /// The mesh this is an instance of
    Mesh* mesh{nullptr};

    /// The transformation matrix for this instance
    Matrix transform;

    /// Tthe material to render this instance with
    Material* material{nullptr};

    /// Material parameters for this instance
    gsl::span<const Param> material_params;
};

} // namespace khepri::renderer
