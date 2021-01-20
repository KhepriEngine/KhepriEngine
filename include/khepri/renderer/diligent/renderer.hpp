#pragma once
#include <khepri/renderer/renderer.hpp>

#include <DeviceContext.h>
#include <RefCntAutoPtr.hpp>
#include <RenderDevice.h>
#include <SwapChain.h>

namespace khepri::renderer::diligent {

class renderable_mesh;
class render_pipeline;
class shader;

/**
 * \brief Diligent-based renderer
 *
 * This renderer uses the Diligent Graphics Engine to render scenes to a surface.
 */
class Renderer : public khepri::renderer::Renderer
{
    struct RenderMesh;
    struct RenderPipeline;

public:
    /**
     * Constructs the Diligent-based renderer.
     *
     * \param[in] window the native window to create the renderer in
     */
    Renderer(const Diligent::NativeWindow& window);
    ~Renderer() override;

    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&)      = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    /**
     * Set the render size for this renderer.
     */
    void render_size(const Size& size);

    /// \see #khepri::renderer::Renderer::render_size
    [[nodiscard]] Size render_size() const noexcept override;

    /// \see #khepri::renderer::Renderer::create_shader
    ShaderId create_shader(const std::filesystem::path& path, ShaderType shader_type,
                           const FileLoader& loader) override;

    /// \see #khepri::renderer::Renderer::create_render_pipeline
    PipelineId create_render_pipeline(const PipelineDesc& desc) override;

    /// \see #khepri::renderer::Renderer::destroy_shader;
    void destroy_shader(ShaderId shader) override;

    /// \see #khepri::renderer::Renderer::destroy_render_pipeline
    void destroy_render_pipeline(PipelineId pipeline) override;

    /// \see #khepri::renderer::Renderer::create_renderable_mesh
    RenderableMeshId create_renderable_mesh(const Mesh& mesh) override;

    /// \see #khepri::renderer::Renderer::destroy_renderable_mesh
    void destroy_renderable_mesh(RenderableMeshId mesh_id) override;

    /// \see #khepri::renderer::Renderer::clear
    void clear() override;

    /// \see #khepri::renderer::Renderer::present
    void present() override;

    /// \see #khepri::renderer::Renderer::render_meshes
    void render_meshes(PipelineId pipeline, gsl::span<const RenderableMeshInstance> meshes,
                       const Camera& camera) override;

private:
    Diligent::RefCntAutoPtr<Diligent::IRenderDevice>  m_device;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_context;
    Diligent::RefCntAutoPtr<Diligent::ISwapChain>     m_swapchain;
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_constants_instance;
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_constants_view;

    std::vector<Diligent::RefCntAutoPtr<Diligent::IShader>> m_shaders;
    std::vector<RenderPipeline>                             m_pipelines;
    std::vector<RenderMesh>                                 m_meshes;
};

} // namespace khepri::renderer::diligent
