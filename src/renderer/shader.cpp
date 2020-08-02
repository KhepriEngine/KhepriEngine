#include <khepri/log/log.hpp>
#include <khepri/renderer/exceptions.hpp>
#include <khepri/renderer/shader.hpp>

#include <shaderc/shaderc.hpp>

namespace khepri::renderer {
namespace {
constexpr khepri::log::Logger LOG("shader");
}

Shader::Shader(gsl::span<const std::uint32_t> data) : m_data(data.begin(), data.end()) {}

Shader Shader::compile(std::string_view shader_source, std::string_view input_name)
{
    shaderc::Compiler       compiler;
    shaderc::CompileOptions options;
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    auto result = compiler.CompileGlslToSpv(shader_source.data(), shader_source.size(),
                                            shaderc_glsl_infer_from_source, input_name.data(),
                                            "main", options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        const auto& error_message = result.GetErrorMessage();
        throw ShaderCompileError("shader compilation failed", error_message);
    }

    const auto* const begin = result.begin();
    const std::size_t size  = result.end() - begin;
    return Shader({begin, size});
}

} // namespace khepri::renderer
