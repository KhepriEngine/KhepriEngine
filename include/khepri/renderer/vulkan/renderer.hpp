#pragma once

#include "detail/device_info.hpp"

#include <khepri/math/size.hpp>
#include <khepri/renderer/renderer.hpp>

#include <vulkan/vulkan.h>

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace khepri::renderer::vulkan {

class RenderableMesh;
class RenderPipeline;

/**
 * \brief Vulkan-based renderer
 *
 * This renderer uses the Vulkan SDK to render scenes to a surface.
 */
class Renderer : public khepri::renderer::Renderer
{
public:
    /**
     * \brief Interface for objects that are capable of providing a rendering surface.
     *
     * The renderer renders scenes to a surface, but it does not own the surface.
     * This interface is passed to the renderer to provide a surface for it to render to.
     */
    class SurfaceProvider
    {
    public:
        SurfaceProvider()          = default;
        virtual ~SurfaceProvider() = default;

        SurfaceProvider(const SurfaceProvider&) = delete;
        SurfaceProvider(SurfaceProvider&&)      = delete;
        SurfaceProvider& operator=(const SurfaceProvider&) = delete;
        SurfaceProvider& operator=(SurfaceProvider&&) = delete;

        /**
         * Returns a list of device extensions that must be supported by a chosen Vulkan device
         * in order to render to surfaces created by this provider.
         */
        [[nodiscard]] virtual std::vector<std::string> required_extensions() const = 0;

        /**
         * Creates a surface that Vulkan can render to.
         *
         * \param[in] instance the vulkan instance to use
         * \param[in] allocator the allocator to use (optional)
         * \param[out] surface the object that will receive the created surface
         * \return the result of the operation
         */
        [[nodiscard]] virtual VkResult create_surface(VkInstance                   instance,
                                                      const VkAllocationCallbacks* allocator,
                                                      VkSurfaceKHR*                surface) = 0;

        /**
         * Returns the size of the render area.
         */
        [[nodiscard]] virtual Size get_render_size() const = 0;
    };

    /**
     * Constructs the Vulkan-based renderer.
     *
     * \param[in] application_name the name of the application as given to Vulkan
     * \param[in] surface_provider an object that can provide a renderable surface
     */
    Renderer(const char* application_name, SurfaceProvider& surface_provider);
    ~Renderer() override;

    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&)      = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    /// \see #khepri::renderer::Renderer::render_size
    [[nodiscard]] Size render_size() const noexcept override;

    /// \see #khepri::renderer::Renderer::create_render_pipeline
    pipeline_id create_render_pipeline(const PipelineDesc& desc) override;

    /// \see #khepri::renderer::Renderer::destroy_render_pipeline
    void destroy_render_pipeline(pipeline_id pipeline) override;

    /// \see #khepri::renderer::Renderer::create_renderable_mesh
    renderable_mesh_id create_renderable_mesh(const Mesh& mesh) override;

    /// \see #khepri::renderer::Renderer::destroy_renderable_mesh
    void destroy_renderable_mesh(renderable_mesh_id mesh_id) override;

    /**
     * Renders a collection of meshes to the surface provided by the surface provider in the
     * constructor.
     *
     * \see #khepri::renderer::Renderer::render_meshes
     */
    void render_meshes(pipeline_id pipeline, gsl::span<const RenderableMeshInstance> meshes,
                       const Camera& camera) override;

private:
    template <typename THandle>
    using UniqueVkInstance = detail::UniqueVkInstance<THandle>;

    template <typename THandle, typename TOwner>
    using UniqueVkHandle = detail::UniqueVkHandle<THandle, TOwner>;

    using GraphicsDeviceInfo = detail::GraphicsDeviceInfo;
    using DeviceInfo         = detail::DeviceInfo;

    struct SwapchainInfo
    {
        UniqueVkHandle<VkSwapchainKHR, VkDevice> handle;
        VkExtent2D                               extent{};

        // Backbuffer image
        VkFormat                                           image_format{};
        std::vector<VkImage>                               images;
        std::vector<UniqueVkHandle<VkImageView, VkDevice>> image_views;

        // Depth buffer
        VkFormat                                 depth_format{};
        UniqueVkHandle<VkImage, VkDevice>        depth_image;
        UniqueVkHandle<VkDeviceMemory, VkDevice> depth_image_memory;
        UniqueVkHandle<VkImageView, VkDevice>    depth_image_view;

        // Command buffers (one per backbuffer image)
        std::vector<VkCommandBuffer> commandbuffers;

        // Shader constants buffers (one per backbuffer image)
        std::vector<UniqueVkHandle<VkBuffer, VkDevice>>       uniform_buffers;
        std::vector<UniqueVkHandle<VkDeviceMemory, VkDevice>> uniform_buffers_memory;
        UniqueVkHandle<VkDescriptorPool, VkDevice>            descriptor_pool;
        std::vector<VkDescriptorSet>                          descriptor_sets;
    };

    static std::optional<GraphicsDeviceInfo>
    find_graphics_device(VkInstance instance, VkSurfaceKHR present_surface,
                         const std::vector<std::string>& required_extensions);

    static DeviceInfo create_device(VkInstance instance, VkSurfaceKHR surface);

    static SwapchainInfo create_swapchain(const DeviceInfo& device, VkCommandPool commandpool,
                                          VkDescriptorSetLayout descriptor_set_layout,
                                          VkSurfaceKHR surface, const Size& surface_size);

    static std::optional<Renderer::GraphicsDeviceInfo>
    get_compatible_device_info(VkPhysicalDevice physical_device, VkSurfaceKHR present_surface,
                               const std::vector<std::string>& required_extensions);

    void recreate_swapchain();
    void recreate_render_pipeline(RenderPipeline& pipeline);

    SurfaceProvider&                                     m_surface_provider;
    UniqueVkInstance<VkInstance>                         m_instance;
    UniqueVkHandle<VkDebugUtilsMessengerEXT, VkInstance> m_debug_messenger;
    detail::DeviceInfo                                   m_device;
    UniqueVkHandle<VkSurfaceKHR, VkInstance>             m_surface;
    UniqueVkHandle<VkCommandPool, VkDevice>              m_commandpool;
    UniqueVkHandle<VkDescriptorSetLayout, VkDevice>      m_descriptor_set_layout;
    SwapchainInfo                                        m_swapchain;

    UniqueVkHandle<VkSemaphore, VkDevice> m_image_available_semaphore;
    UniqueVkHandle<VkSemaphore, VkDevice> m_render_finished_semaphore;

    std::vector<std::unique_ptr<RenderableMesh>> m_renderable_meshes;
    std::vector<std::unique_ptr<RenderPipeline>> m_render_pipelines;
};

} // namespace khepri::renderer::vulkan
