#pragma once
#include <khepri/renderer/renderer.hpp>

#include <DeviceContext.h>
#include <RefCntAutoPtr.hpp>
#include <RenderDevice.h>
#include <SwapChain.h>

namespace khepri::renderer::diligent {

/**
 * \brief Diligent-based renderer
 *
 * This renderer uses the Diligent Graphics Engine to render scenes to a surface.
 */
class Renderer : public khepri::renderer::Renderer
{
    struct Shader;
    struct Material;
    struct Texture;
    struct Mesh;

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
    std::unique_ptr<khepri::renderer::Shader> create_shader(const std::filesystem::path& path,
                                                            const ShaderLoader& loader) override;

    /// \see #khepri::renderer::Renderer::create_material
    std::unique_ptr<khepri::renderer::Material>
    create_material(const khepri::renderer::MaterialDesc& material_desc) override;

    /// \see #khepri::renderer::Renderer::create_texture;
    std::unique_ptr<khepri::renderer::Texture>
    create_texture(const khepri::renderer::TextureDesc& texture_desc) override;

    /// \see #khepri::renderer::Renderer::create_mesh
    std::unique_ptr<khepri::renderer::Mesh>
    create_mesh(const khepri::renderer::MeshDesc& mesh_desc) override;

    /// \see #khepri::renderer::Renderer::clear
    void clear(ClearFlags flags) override;

    /// \see #khepri::renderer::Renderer::present
    void present() override;

    /// \see #khepri::renderer::Renderer::render_meshes
    void render_meshes(gsl::span<const MeshInstance> meshes, const Camera& camera) override;

    /// \see #khepri::renderer::Renderer::render_sprites
    void render_sprites(gsl::span<const Sprite> sprites, khepri::renderer::Material& material,
                        gsl::span<const khepri::renderer::Material::Param> params) override;

private:
    // Number of vertices to render one sprite
    static constexpr std::size_t VERTICES_PER_SPRITE = 4;

    // Number of sprites that fit in the sprite vertex/index buffers
    static constexpr std::size_t SPRITE_BUFFER_COUNT = 1024;

    using SpriteVertex = MeshDesc::Vertex;

    void apply_material_params(Material&                                          material,
                               gsl::span<const khepri::renderer::Material::Param> params);

    static std::vector<std::string>
    determine_dynamic_material_variables(const Shader&                              shader,
                                         const std::vector<MaterialDesc::Property>& properties);

    Diligent::RefCntAutoPtr<Diligent::IRenderDevice>  m_device;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_context;
    Diligent::RefCntAutoPtr<Diligent::ISwapChain>     m_swapchain;
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_constants_instance;
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_constants_view;
    Diligent::RefCntAutoPtr<Diligent::ISampler>       m_linear_sampler;
    Diligent::RefCntAutoPtr<Diligent::ISampler>       m_linear_clamp_sampler;
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_sprite_vertex_buffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_sprite_index_buffer;
};

} // namespace khepri::renderer::diligent
