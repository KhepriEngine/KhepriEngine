#pragma once
#include <khepri/renderer/renderer.hpp>

#include <DeviceContext.h>
#include <RefCntAutoPtr.hpp>
#include <RenderDevice.h>
#include <SwapChain.h>
#include <unordered_map>

namespace khepri::renderer::diligent {

/**
 * \brief Diligent-based renderer
 *
 * This renderer uses the Diligent Graphics Engine to render scenes to a surface.
 */
class Renderer : public khepri::renderer::Renderer
{
    struct ShaderData;
    struct MaterialData;
    struct TextureData;
    struct MeshData;

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
    ShaderId create_shader(const std::filesystem::path& path, const FileLoader& loader) override;

    /// \see #khepri::renderer::Renderer::destroy_shader;
    void destroy_shader(ShaderId shader_id) override;

    /// \see #khepri::renderer::Renderer::create_material
    MaterialId create_material(const khepri::renderer::Material& material) override;

    /// \see #khepri::renderer::Renderer::destroy_material
    void destroy_material(MaterialId material_id) override;

    /// \see #khepri::renderer::Renderer::create_texture;
    TextureId create_texture(const khepri::renderer::Texture& texture) override;

    /// \see #khepri::renderer::Renderer::destroy_texture;
    void destroy_texture(TextureId texture_id) override;

    /// \see #khepri::renderer::Renderer::create_mesh
    MeshId create_mesh(const khepri::renderer::Mesh& mesh) override;

    /// \see #khepri::renderer::Renderer::destroy_mesh
    void destroy_mesh(MeshId mesh_id) override;

    /// \see #khepri::renderer::Renderer::clear
    void clear() override;

    /// \see #khepri::renderer::Renderer::present
    void present() override;

    /// \see #khepri::renderer::Renderer::render_meshes
    void render_meshes(gsl::span<const MeshInstance> meshes, const Camera& camera) override;

private:
    void apply_material_params(MaterialData& material, gsl::span<const MeshInstance::Param> params);

    static std::vector<std::string>
    determine_dynamic_material_variables(const ShaderData&                      shader,
                                         const std::vector<Material::Property>& properties);

    Diligent::RefCntAutoPtr<Diligent::IRenderDevice>  m_device;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_context;
    Diligent::RefCntAutoPtr<Diligent::ISwapChain>     m_swapchain;
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_constants_instance;
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_constants_view;
    Diligent::RefCntAutoPtr<Diligent::ISampler>       m_linear_sampler;

    std::vector<ShaderData>   m_shaders;
    std::vector<MaterialData> m_materials;
    std::vector<TextureData>  m_textures;
    std::vector<MeshData>     m_meshes;
};

} // namespace khepri::renderer::diligent
