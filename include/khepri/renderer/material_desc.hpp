#pragma once

#include "shader.hpp"
#include "texture.hpp"

#include <khepri/math/matrix.hpp>
#include <khepri/math/vector2.hpp>
#include <khepri/math/vector3.hpp>
#include <khepri/math/vector4.hpp>

#include <string>
#include <variant>
#include <vector>

namespace khepri::renderer {

/**
 * \brief Description of a material
 *
 * A material is defined by a collection of shaders and a collection of related properties
 * (integers, floats, vectors, matrices and textures) that can be passed into a shader when
 * rendering a mesh.
 *
 * The properties in a material can be set by name in a #khepri::renderer::MeshInstance when passed
 * to the renderer. The specified (non-texture) properties are set in the shader's constant buffer
 * with name @c Material (if present). Texture properties are set on the shader's texture resource
 * with the same name directly.
 */
struct MaterialDesc
{
    /// The type of face culling
    enum class CullMode
    {
        none,
        back,
        front,
    };

    /// The type of alpha blending
    enum class AlphaBlendMode
    {
        /// Do not alpha blend.
        none,

        /// Source and destination are blended according to source alpha.
        blend_src,

        /// Source is added on top of destination.
        additive,
    };

    /// Value of a material shader property
    using PropertyValue =
        std::variant<std::int32_t, float, Vector2f, Vector3f, Vector4f, Matrixf, Texture*>;

    /// Description of a material shader property
    struct Property
    {
        /// Property name
        std::string name;

        /// Default value of the property if none is provided by the mesh instance; this also
        /// determines the property's type
        PropertyValue default_value;
    };

    /// Face culling mode of this material
    CullMode cull_mode{CullMode::none};

    /// Type of alpha blending to use when rendering with this material
    AlphaBlendMode alpha_blend_mode{AlphaBlendMode::none};

    /// Enable depth-buffer test when rendering this material
    bool depth_enable{true};

    /// Shader of this material
    Shader* shader{nullptr};

    /// Shader properties of this material
    std::vector<Property> properties;
};

} // namespace khepri::renderer
