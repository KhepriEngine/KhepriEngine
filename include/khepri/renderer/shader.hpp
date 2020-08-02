#pragma once

#include <gsl/gsl-lite.hpp>

#include <cstdint>
#include <string_view>
#include <vector>

namespace khepri::renderer {

/**
 * \brief Represents a compiled shader
 */
class Shader
{
public:
    /**
     * \brief Compiles GLSL sources to a shader.
     *
     * \param[in] shader_source the GLSL source code to compile
     * \param[in] input_name the name of the input, for error reporting
     *
     * \return the shader
     *
     * \throw khepri::renderer::error if the shader cannot be compiled
     */
    static Shader compile(std::string_view shader_source, std::string_view input_name);

    /**
     * \brief Constructs a shader out of SPIR-V byte code
     *
     * \param[in] spirv_data the SPIR-V byte code
     */
    explicit Shader(gsl::span<const std::uint32_t> spirv_data);

    /**
     * Returns the shader's SPIR-V data.
     */
    [[nodiscard]] gsl::span<const std::uint32_t> data() const noexcept
    {
        return m_data;
    }

private:
    std::vector<std::uint32_t> m_data;
};

} // namespace khepri::renderer
