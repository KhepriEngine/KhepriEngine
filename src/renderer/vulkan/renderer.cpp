#include "exceptions.hpp"
#include "logging.hpp"
#include "render_pipeline.hpp"
#include "renderable_mesh.hpp"

#include <khepri/log/log.hpp>
#include <khepri/renderer/vulkan/renderer.hpp>

#include <gsl/gsl-lite.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

namespace khepri::renderer::vulkan {

using detail::DeviceInfo;
using detail::GraphicsDeviceInfo;
using detail::make_unique_vk_handle;
using detail::UniqueVkHandle;

namespace {
template <typename VulkanFunc>
auto get_vulkan_instance_proc(VkInstance instance, const char* name)
{
    // NOLINTNEXTLINE -- this reinterpret cast is necessary, unfortunately
    return reinterpret_cast<VulkanFunc>(vkGetInstanceProcAddr(instance, name));
}

const char* get_device_type_string(VkPhysicalDeviceType device_type)
{
    switch (device_type) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        return "Other";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return "Integrated GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return "Discrete GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return "Virtual GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return "CPU";
    case VK_PHYSICAL_DEVICE_TYPE_RANGE_SIZE:
    case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
        break;
    }
    assert(false);
    return "Unknown";
}

const char* get_queue_family_type(const VkQueueFamilyProperties& queue_family)
{
    if ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
        if ((queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) {
            return "graphics & compute";
        }
        return "graphics";
    }

    if ((queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) {
        return "compute";
    }

    if ((queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) {
        return "transfer";
    }

    return nullptr;
}

auto to_const_char_vector(const std::vector<std::string>& strings)
{
    using namespace std::placeholders;

    std::vector<const char*> results(strings.size());
    std::transform(strings.begin(), strings.end(), results.begin(),
                   [](const auto& str) { return str.c_str(); });
    return results;
}

bool device_supports_extensions(VkPhysicalDevice                device,
                                const std::vector<std::string>& required_extensions)
{
    std::uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> device_extensions(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, device_extensions.data());

    const auto& device_supports_extension = [&device_extensions](const auto& required_extension) {
        return std::any_of(device_extensions.begin(), device_extensions.end(),
                           [&required_extension](const auto& extension) {
                               return required_extension == &extension.extensionName[0];
                           });
    };

    return std::all_of(required_extensions.begin(), required_extensions.end(),
                       device_supports_extension);
}

auto get_physical_device_queue_family_properties(VkPhysicalDevice device)
{
    std::uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> properties(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, properties.data());
    return properties;
}

auto get_physical_device_surface_formats(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    std::uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data());
    return formats;
}

auto get_physical_device_surface_present_modes(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    std::uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data());
    return modes;
}

auto find_memory_type(const DeviceInfo& device, uint32_t type_filter,
                      VkMemoryPropertyFlags properties)
{
    const auto memory_types = gsl::span<const VkMemoryType>(&device.mem_properties.memoryTypes[0],
                                                            device.mem_properties.memoryTypeCount);

    for (std::uint32_t i = 0; i < memory_types.size(); i++) {
        if (((type_filter & (1 << i)) != 0) &&
            (memory_types[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw Error("failed to find suitable memory type");
}

VkSurfaceFormatKHR
pick_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) noexcept
{
    assert(!available_formats.empty());

    // The best option is a 32-bit sRGB format
    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    // Last resort: pick the first
    return available_formats[0];
}

VkPresentModeKHR
pick_swapchain_present_mode(const std::vector<VkPresentModeKHR>& available_modes) noexcept
{
    // We prefer Mailbox, it's the best latency-tearing tradeoff
    if (std::find(available_modes.begin(), available_modes.end(), VK_PRESENT_MODE_MAILBOX_KHR) !=
        available_modes.end()) {
        return VK_PRESENT_MODE_MAILBOX_KHR;
    }

    // FIFO is guaranteed to be available
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D pick_swapchain_extent(const VkSurfaceCapabilitiesKHR& surface_capabilities,
                                 const VkExtent2D&               desired_extent) noexcept
{
    if (surface_capabilities.currentExtent.width != UINT32_MAX) {
        // The extent is known from the surface
        return surface_capabilities.currentExtent;
    }

    // We have to pick an extent
    VkExtent2D extent = desired_extent;
    extent.width      = std::max(surface_capabilities.minImageExtent.width,
                            std::min(surface_capabilities.maxImageExtent.width, extent.width));
    extent.height     = std::max(surface_capabilities.minImageExtent.height,
                             std::min(surface_capabilities.maxImageExtent.height, extent.height));
    return extent;
}

VkFormat pick_supported_format(const DeviceInfo& device, const std::vector<VkFormat>& candidates,
                               VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(device.graphics_device.physical_device, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        }

        if (tiling == VK_IMAGE_TILING_OPTIMAL &&
            (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw Error("no supported depth image format");
}

VkFormat pick_depthimage_format(const DeviceInfo& device)
{
    return pick_supported_format(
        device, {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

std::pair<UniqueVkHandle<VkImage, VkDevice>, UniqueVkHandle<VkDeviceMemory, VkDevice>>
create_image(const DeviceInfo& device, std::uint32_t width, std::uint32_t height, VkFormat format,
             VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
{
    VkImageCreateInfo image_info{};
    image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType     = VK_IMAGE_TYPE_2D;
    image_info.extent.width  = width;
    image_info.extent.height = height;
    image_info.extent.depth  = 1;
    image_info.mipLevels     = 1;
    image_info.arrayLayers   = 1;
    image_info.format        = format;
    image_info.tiling        = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage         = usage;
    image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    VkImage raw_image{};
    verify(vkCreateImage(device.device, &image_info, nullptr, &raw_image));
    auto image = make_unique_vk_handle(raw_image, device.device, nullptr, vkDestroyImage);

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(device.device, image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex =
        find_memory_type(device, mem_requirements.memoryTypeBits, properties);

    VkDeviceMemory raw_image_memory{};
    verify(vkAllocateMemory(device.device, &alloc_info, nullptr, &raw_image_memory));
    auto image_memory =
        make_unique_vk_handle(raw_image_memory, device.device, nullptr, vkFreeMemory);

    vkBindImageMemory(device.device, image, image_memory, 0);

    return {std::move(image), std::move(image_memory)};
}

auto create_image_view(VkDevice device, VkImage image, VkFormat format,
                       VkImageAspectFlags aspect_mask)
{
    VkImageViewCreateInfo view_info{};
    view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image                           = image;
    view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format                          = format;
    view_info.subresourceRange.aspectMask     = aspect_mask;
    view_info.subresourceRange.baseMipLevel   = 0;
    view_info.subresourceRange.levelCount     = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount     = 1;

    VkImageView raw_image_view{};
    verify(vkCreateImageView(device, &view_info, nullptr, &raw_image_view));
    return make_unique_vk_handle(raw_image_view, device, nullptr, vkDestroyImageView);
}

auto get_swapchain_images(VkDevice device, VkSwapchainKHR swapchain)
{
    std::uint32_t count = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
    std::vector<VkImage> images(count);
    vkGetSwapchainImagesKHR(device, swapchain, &count, images.data());
    return images;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* /*pUserData*/)
{
    log::severity severity = log::severity::info;
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        severity = log::severity::debug;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        severity = log::severity::info;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        severity = log::severity::warning;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        severity = log::severity::error;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
        assert(false);
        break;
    }

    LOG.log(severity, "{}", pCallbackData->pMessage);
    return VK_FALSE;
}

auto create_semaphore(VkDevice device)
{
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore raw_semaphore{};
    verify(vkCreateSemaphore(device, &semaphore_info, nullptr, &raw_semaphore));
    return make_unique_vk_handle(raw_semaphore, device, nullptr, vkDestroySemaphore);
}

template <typename TContainer>
auto create_buffer(const DeviceInfo& device, const TContainer& data, VkBufferUsageFlags usage)
{
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size        = sizeof(decltype(data[0])) * data.size();
    buffer_info.usage       = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    UniqueVkHandle<VkBuffer, VkDevice> buffer;
    {
        VkBuffer raw_buffer{};
        verify(vkCreateBuffer(device.device, &buffer_info, nullptr, &raw_buffer));
        buffer = make_unique_vk_handle(raw_buffer, device.device, nullptr, vkDestroyBuffer);
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(device.device, buffer, &mem_requirements);

    UniqueVkHandle<VkDeviceMemory, VkDevice> buffer_memory;
    {
        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize  = mem_requirements.size;
        alloc_info.memoryTypeIndex = find_memory_type(device, mem_requirements.memoryTypeBits,
                                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkDeviceMemory raw_buffer_memory{};
        verify(vkAllocateMemory(device.device, &alloc_info, nullptr, &raw_buffer_memory));
        buffer_memory =
            make_unique_vk_handle(raw_buffer_memory, device.device, nullptr, vkFreeMemory);
    }

    vkBindBufferMemory(device.device, buffer, buffer_memory, 0);

    void* buffer_data{};
    vkMapMemory(device.device, buffer_memory, 0, buffer_info.size, 0, &buffer_data);
    std::memcpy(buffer_data, data.data(), (size_t)buffer_info.size);
    vkUnmapMemory(device.device, buffer_memory);

    return std::make_tuple(std::move(buffer), std::move(buffer_memory));
}

} // namespace

Renderer::Renderer(const char* application_name, SurfaceProvider& surface_provider)
    : m_surface_provider(surface_provider)
{
    // Select the Vulkan API
    VkApplicationInfo appInfo  = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = application_name;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "Khepri";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    std::uint32_t apiVersion = 0;
    verify(vkEnumerateInstanceVersion(&apiVersion));
    LOG.info("Vulkan implementation supports version {}.{}.{}, selecting {}.{}.{}",
             VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion),
             VK_VERSION_PATCH(apiVersion), VK_VERSION_MAJOR(appInfo.apiVersion),
             VK_VERSION_MINOR(appInfo.apiVersion), VK_VERSION_PATCH(appInfo.apiVersion));

    // Construct the instance
    {
        // Construct the list of layers and extensions
        std::vector<std::string> layers;
        std::vector<std::string> extensions = surface_provider.required_extensions();

#ifndef NDEBUG
        // In debug builds, add the validation layer and debug extensions
        layers.emplace_back("VK_LAYER_KHRONOS_validation");
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        const auto vk_extensions = to_const_char_vector(extensions);
        const auto vk_layers     = to_const_char_vector(layers);

        VkInstanceCreateInfo create_info    = {};
        create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo        = &appInfo;
        create_info.enabledExtensionCount   = static_cast<std::uint32_t>(vk_extensions.size());
        create_info.ppEnabledExtensionNames = vk_extensions.data();
        create_info.enabledLayerCount       = static_cast<std::uint32_t>(vk_layers.size());
        create_info.ppEnabledLayerNames     = vk_layers.data();

        VkInstance instance{};
        verify(vkCreateInstance(&create_info, nullptr, &instance));
        m_instance = UniqueVkInstance<VkInstance>(instance, nullptr, &vkDestroyInstance);
    }

#ifndef NDEBUG
    {
        const auto vkCreateDebugUtilsMessengerEXT =
            get_vulkan_instance_proc<PFN_vkCreateDebugUtilsMessengerEXT>(
                m_instance, "vkCreateDebugUtilsMessengerEXT");

        const auto vkDestroyDebugUtilsMessengerEXT =
            get_vulkan_instance_proc<PFN_vkDestroyDebugUtilsMessengerEXT>(
                m_instance, "vkDestroyDebugUtilsMessengerEXT");

        if (vkCreateDebugUtilsMessengerEXT != nullptr &&
            vkDestroyDebugUtilsMessengerEXT != nullptr) {
            // Create the debug layer
            VkDebugUtilsMessengerCreateInfoEXT create_info = {};
            create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            create_info.pfnUserCallback = vulkan_debug_callback;

            VkDebugUtilsMessengerEXT debug_messenger{};
            verify(vkCreateDebugUtilsMessengerEXT(m_instance, &create_info, nullptr,
                                                  &debug_messenger));
            m_debug_messenger = make_unique_vk_handle(debug_messenger, m_instance, nullptr,
                                                      vkDestroyDebugUtilsMessengerEXT);
        }
    }
#endif

    // Obtain the WSI surface
    VkSurfaceKHR surface{};
    verify(surface_provider.create_surface(m_instance, nullptr, &surface));
    m_surface = make_unique_vk_handle(surface, m_instance, nullptr, &vkDestroySurfaceKHR);

    // Create the logical device for the surface
    m_device = create_device(m_instance, m_surface);

    // Create a command pool for the device
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = m_device.graphics_device.graphics_queue_family_index;
        poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VkCommandPool raw_commandpool{};
        verify(vkCreateCommandPool(m_device.device, &poolInfo, nullptr, &raw_commandpool));
        m_commandpool =
            make_unique_vk_handle(raw_commandpool, m_device.device, nullptr, vkDestroyCommandPool);
    }

    // Create the global descriptor set
    {
        VkDescriptorSetLayoutBinding layout_binding{};
        layout_binding.binding            = 0;
        layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layout_binding.descriptorCount    = 1;
        layout_binding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
        layout_binding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layout_info{};
        layout_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = 1;
        layout_info.pBindings    = &layout_binding;

        VkDescriptorSetLayout descriptor_set_layout{};
        verify(vkCreateDescriptorSetLayout(m_device.device, &layout_info, nullptr,
                                           &descriptor_set_layout));
        m_descriptor_set_layout = make_unique_vk_handle(descriptor_set_layout, m_device.device,
                                                        nullptr, vkDestroyDescriptorSetLayout);
    }

    m_image_available_semaphore = create_semaphore(m_device.device);
    m_render_finished_semaphore = create_semaphore(m_device.device);

    // Create the swap chain
    const auto surface_size = surface_provider.get_render_size();
    m_swapchain =
        create_swapchain(m_device, m_commandpool, m_descriptor_set_layout, m_surface, surface_size);

    LOG.info("Created {}px x {}px swapchain with {} images", m_swapchain.extent.width,
             m_swapchain.extent.height, m_swapchain.images.size());
}

Renderer::~Renderer()
{
    // Always wait until the device is idle
    vkDeviceWaitIdle(m_device.device);
}

Size Renderer::render_size() const noexcept
{
    return {m_swapchain.extent.width, m_swapchain.extent.height};
}

Renderer::DeviceInfo Renderer::create_device(VkInstance instance, VkSurfaceKHR surface)
{
    // Enumerate physical device groups and find the physical devices that can render to the
    // obtained surface
    const std::vector<std::string> device_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    const auto graphics_device = find_graphics_device(instance, surface, device_extensions);
    if (!graphics_device) {
        throw Error("No valid graphics devices found");
    }

    DeviceInfo info;
    info.graphics_device = *graphics_device;

    // Create logical device from the found physical device
    {
        // We only need one queue
        const std::array<float, 1> queue_priorities{1.0F};

        const std::set<std::uint32_t> unique_queue_family_indices = {
            info.graphics_device.graphics_queue_family_index,
            info.graphics_device.present_queue_family_index};

        std::vector<VkDeviceQueueCreateInfo> queue_infos;
        for (const auto queue_family_index : unique_queue_family_indices) {
            VkDeviceQueueCreateInfo queue_info{};
            queue_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info.queueFamilyIndex = queue_family_index;
            queue_info.queueCount       = static_cast<std::uint32_t>(queue_priorities.size());
            queue_info.pQueuePriorities = queue_priorities.data();
            queue_infos.push_back(queue_info);
        }

        const auto vk_device_extensions = to_const_char_vector(device_extensions);

        VkDeviceCreateInfo create_info    = {};
        create_info.sType                 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount  = static_cast<std::uint32_t>(queue_infos.size());
        create_info.pQueueCreateInfos     = queue_infos.data();
        create_info.enabledExtensionCount = static_cast<std::uint32_t>(vk_device_extensions.size());
        create_info.ppEnabledExtensionNames = vk_device_extensions.data();

        VkDevice device{};
        verify(
            vkCreateDevice(info.graphics_device.physical_device, &create_info, nullptr, &device));
        info.device = UniqueVkInstance<VkDevice>(device, nullptr, vkDestroyDevice);
    }

    vkGetPhysicalDeviceMemoryProperties(info.graphics_device.physical_device, &info.mem_properties);
    vkGetDeviceQueue(info.device, info.graphics_device.graphics_queue_family_index, 0,
                     &info.graphics_queue);
    vkGetDeviceQueue(info.device, info.graphics_device.present_queue_family_index, 0,
                     &info.present_queue);
    return info;
}

std::optional<Renderer::GraphicsDeviceInfo>
Renderer::get_compatible_device_info(VkPhysicalDevice physical_device, VkSurfaceKHR present_surface,
                                     const std::vector<std::string>& required_extensions)
{
    Renderer::GraphicsDeviceInfo device_info = {};

    device_info.physical_device = physical_device;

    const bool supports_extensions =
        device_supports_extensions(device_info.physical_device, required_extensions);

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device_info.physical_device, &properties);

    device_info.surface_formats =
        get_physical_device_surface_formats(device_info.physical_device, present_surface);

    device_info.present_modes =
        get_physical_device_surface_present_modes(device_info.physical_device, present_surface);

    const auto queue_families =
        get_physical_device_queue_family_properties(device_info.physical_device);

    bool has_graphics     = false;
    bool supports_surface = false;

    LOG.info("- {} \"{}\" ({:08x}:{:08x}), API {}.{}.{}, driver {:08x}{}:",
             get_device_type_string(properties.deviceType), properties.deviceName,
             properties.vendorID, properties.deviceID, VK_VERSION_MAJOR(properties.apiVersion),
             VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion),
             properties.driverVersion, supports_extensions ? " (extensions supported)" : "");

    for (std::uint32_t idx_family = 0; idx_family < queue_families.size(); ++idx_family) {
        const auto&       queue_family = queue_families[idx_family];
        const auto* const queue_type   = get_queue_family_type(queue_family);

        if ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            device_info.graphics_queue_family_index = idx_family;
            device_info.graphics_queue_count        = queue_family.queueCount;
            has_graphics                            = true;
        }

        VkBool32 present_supported{VK_FALSE};
        verify(vkGetPhysicalDeviceSurfaceSupportKHR(device_info.physical_device, idx_family,
                                                    present_surface, &present_supported));
        if (present_supported != VK_FALSE) {
            device_info.present_queue_family_index = idx_family;
            device_info.present_queue_count        = queue_family.queueCount;
            supports_surface                       = true;
        }

        if (queue_type != nullptr) {
            LOG.info("  - {:2} {} queues{}", queue_family.queueCount, queue_type,
                     supports_surface ? " (can present)" : "");
        }
    }

    if (has_graphics && supports_surface && supports_extensions &&
        !device_info.surface_formats.empty() && !device_info.present_modes.empty()) {
        return device_info;
    }
    return {};
}

// Enumerate physical devices on the instance and find a physical device with
// graphics capabilities that can render to the specified surface and has the required device
// extensions.
std::optional<Renderer::GraphicsDeviceInfo>
Renderer::find_graphics_device(VkInstance instance, VkSurfaceKHR present_surface,
                               const std::vector<std::string>& required_extensions)
{
    std::uint32_t count = 0;
    verify(vkEnumeratePhysicalDeviceGroups(instance, &count, nullptr));

    std::vector<VkPhysicalDeviceGroupProperties> groups(count);
    verify(vkEnumeratePhysicalDeviceGroups(instance, &count, groups.data()));

    std::vector<GraphicsDeviceInfo> graphics_devices;
    for (std::uint32_t idxGroup = 0; idxGroup < groups.size(); ++idxGroup) {
        const auto& group = groups[idxGroup];
        const auto  physical_devices =
            gsl::span<const VkPhysicalDevice>{&group.physicalDevices[0], group.physicalDeviceCount};

        LOG.info("Device group {}: ", idxGroup);
        for (const auto& physical_device : physical_devices) {
            auto device_info =
                get_compatible_device_info(physical_device, present_surface, required_extensions);

            if (device_info) {
                graphics_devices.push_back(std::move(*device_info));
            }
        }
    }

    if (graphics_devices.empty()) {
        return {};
    }
    return *graphics_devices.begin();
}

Renderer::SwapchainInfo Renderer::create_swapchain(const DeviceInfo&     device,
                                                   VkCommandPool         commandpool,
                                                   VkDescriptorSetLayout descriptor_set_layout,
                                                   VkSurfaceKHR surface, const Size& surface_size)
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    verify(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.graphics_device.physical_device,
                                                     surface, &surface_capabilities));

    const auto surface_format =
        pick_swapchain_surface_format(device.graphics_device.surface_formats);
    const auto present_mode = pick_swapchain_present_mode(device.graphics_device.present_modes);

    SwapchainInfo swapchain_info;

    {
        // Try to at least double buffer
        uint32_t image_count = surface_capabilities.minImageCount + 1;
        if (surface_capabilities.maxImageCount > 0) {
            image_count = std::min(image_count, surface_capabilities.maxImageCount);
        }

        VkSwapchainCreateInfoKHR create_info{};
        create_info.sType           = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface         = surface;
        create_info.minImageCount   = image_count;
        create_info.imageFormat     = surface_format.format;
        create_info.imageColorSpace = surface_format.colorSpace;
        create_info.imageExtent     = pick_swapchain_extent(
            surface_capabilities, {static_cast<std::uint32_t>(surface_size.width),
                                   static_cast<std::uint32_t>(surface_size.height)});
        create_info.imageArrayLayers = 1; // Non-stereoscopic
        create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        create_info.preTransform     = surface_capabilities.currentTransform;
        create_info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode      = present_mode;
        create_info.clipped          = VK_TRUE;

        const std::array<std::uint32_t, 2> queue_family_indices{
            device.graphics_device.graphics_queue_family_index,
            device.graphics_device.present_queue_family_index};

        if (device.graphics_device.graphics_queue_family_index !=
            device.graphics_device.present_queue_family_index) {
            // We're sharing the swap chain
            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount =
                static_cast<std::uint32_t>(queue_family_indices.size());
            create_info.pQueueFamilyIndices = queue_family_indices.data();
        } else {
            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        VkSwapchainKHR swapchain{};
        verify(vkCreateSwapchainKHR(device.device, &create_info, nullptr, &swapchain));
        swapchain_info.handle =
            make_unique_vk_handle(swapchain, device.device, nullptr, vkDestroySwapchainKHR);
        swapchain_info.extent       = create_info.imageExtent;
        swapchain_info.image_format = create_info.imageFormat;
    }

    swapchain_info.images = get_swapchain_images(device.device, swapchain_info.handle);

    swapchain_info.image_views.resize(swapchain_info.images.size());
    for (std::size_t i = 0; i < swapchain_info.images.size(); i++) {
        swapchain_info.image_views[i] =
            create_image_view(device.device, swapchain_info.images[i], swapchain_info.image_format,
                              VK_IMAGE_ASPECT_COLOR_BIT);
    }

    // Create depth buffer
    {
        swapchain_info.depth_format = pick_depthimage_format(device);

        std::tie(swapchain_info.depth_image, swapchain_info.depth_image_memory) = create_image(
            device, swapchain_info.extent.width, swapchain_info.extent.height,
            swapchain_info.depth_format, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        swapchain_info.depth_image_view =
            create_image_view(device.device, swapchain_info.depth_image,
                              swapchain_info.depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    // Allocate a command buffer per image
    {
        swapchain_info.commandbuffers.resize(swapchain_info.image_views.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool        = commandpool;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)swapchain_info.commandbuffers.size();
        verify(vkAllocateCommandBuffers(device.device, &allocInfo,
                                        swapchain_info.commandbuffers.data()));
    }

    // Allocate a uniform buffer per image
    {
        // Global data is irrelevant at this point, we'll overwrite this every frame anyway
        PipelineGlobals globals{};

        swapchain_info.uniform_buffers.resize(swapchain_info.image_views.size());
        swapchain_info.uniform_buffers_memory.resize(swapchain_info.image_views.size());

        for (size_t i = 0; i < swapchain_info.image_views.size(); i++) {
            std::tie(swapchain_info.uniform_buffers[i], swapchain_info.uniform_buffers_memory[i]) =
                create_buffer(device, gsl::span<PipelineGlobals>{globals},
                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        }
    }

    {
        VkDescriptorPoolSize pool_size{};
        pool_size.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_size.descriptorCount = static_cast<uint32_t>(swapchain_info.image_views.size());

        VkDescriptorPoolCreateInfo pool_info{};
        pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.poolSizeCount = 1;
        pool_info.pPoolSizes    = &pool_size;
        pool_info.maxSets       = static_cast<uint32_t>(swapchain_info.image_views.size());

        VkDescriptorPool descriptor_pool{};
        verify(vkCreateDescriptorPool(device.device, &pool_info, nullptr, &descriptor_pool));
        swapchain_info.descriptor_pool =
            make_unique_vk_handle(descriptor_pool, device.device, nullptr, vkDestroyDescriptorPool);
    }

    {
        std::vector<VkDescriptorSetLayout> layouts(swapchain_info.image_views.size(),
                                                   descriptor_set_layout);

        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool     = swapchain_info.descriptor_pool;
        alloc_info.descriptorSetCount = static_cast<uint32_t>(swapchain_info.image_views.size());
        alloc_info.pSetLayouts        = layouts.data();

        swapchain_info.descriptor_sets.resize(swapchain_info.image_views.size());
        verify(vkAllocateDescriptorSets(device.device, &alloc_info,
                                        swapchain_info.descriptor_sets.data()));
    }

    for (size_t i = 0; i < swapchain_info.image_views.size(); i++) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = swapchain_info.uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range  = sizeof(PipelineGlobals);

        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet           = swapchain_info.descriptor_sets[i];
        descriptor_write.dstBinding       = 0;
        descriptor_write.dstArrayElement  = 0;
        descriptor_write.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount  = 1;
        descriptor_write.pBufferInfo      = &buffer_info;
        descriptor_write.pImageInfo       = nullptr;
        descriptor_write.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets(device.device, 1, &descriptor_write, 0, nullptr);
    }

    return swapchain_info;
}

void Renderer::recreate_swapchain()
{
    // Don't release any resources anything until the device is idle
    vkDeviceWaitIdle(m_device.device);

    // Clear the old pipelines
    for (auto& pipeline : m_render_pipelines) {
        pipeline->framebuffers.clear();
        pipeline->pipeline    = {};
        pipeline->render_pass = {};
    }

    // Release the old swapchain resources
    m_swapchain = {};

    // Create the new swapchain
    const auto surface_size = m_surface_provider.get_render_size();
    m_swapchain =
        create_swapchain(m_device, m_commandpool, m_descriptor_set_layout, m_surface, surface_size);
    LOG.info("Recreated {}px x {}px swapchain with {} images", m_swapchain.extent.width,
             m_swapchain.extent.height, m_swapchain.images.size());

    // Recreate all pipeline resources
    for (auto& pipeline : m_render_pipelines) {
        recreate_render_pipeline(*pipeline);
    }
}

renderable_mesh_id Renderer::create_renderable_mesh(const Mesh& mesh)
{
    auto it = std::find(m_renderable_meshes.begin(), m_renderable_meshes.end(), nullptr);
    if (it == m_renderable_meshes.end()) {
        it = m_renderable_meshes.insert(m_renderable_meshes.end(), nullptr);
    }

    *it = std::make_unique<RenderableMesh>();

    // Create the vertex buffer
    std::tie((*it)->vertex_buffer, (*it)->vertex_buffer_memory) =
        create_buffer(m_device, mesh.vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    (*it)->vertex_count = static_cast<std::uint32_t>(mesh.vertices.size());

    // Create the index buffer
    static_assert(sizeof(mesh.indices[0]) == sizeof(std::uint16_t));
    std::tie((*it)->index_buffer, (*it)->index_buffer_memory) =
        create_buffer(m_device, mesh.indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    (*it)->index_count = static_cast<std::uint32_t>(mesh.indices.size());

    return it - m_renderable_meshes.begin();
}

void Renderer::destroy_renderable_mesh(renderable_mesh_id mesh_id)
{
    assert(mesh_id < m_renderable_meshes.size());
    assert(m_renderable_meshes[mesh_id] != nullptr);

    m_renderable_meshes[mesh_id] = nullptr;
}

pipeline_id Renderer::create_render_pipeline(const PipelineDesc& desc)
{
    const auto& load_shader = [this](const khepri::renderer::Shader* shader) {
        UniqueVkHandle<VkShaderModule, VkDevice> shader_module;
        if (shader != nullptr) {
            VkShaderModuleCreateInfo create_info{};
            create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            create_info.pCode    = shader->data().data();
            create_info.codeSize = shader->data().size() * sizeof(std::uint32_t);

            VkShaderModule raw_shader_module{};
            verify(
                vkCreateShaderModule(m_device.device, &create_info, nullptr, &raw_shader_module));
            shader_module = make_unique_vk_handle(raw_shader_module, m_device.device, nullptr,
                                                  vkDestroyShaderModule);
        }
        return shader_module;
    };

    auto it = std::find(m_render_pipelines.begin(), m_render_pipelines.end(), nullptr);
    if (it == m_render_pipelines.end()) {
        it = m_render_pipelines.insert(m_render_pipelines.end(), nullptr);
    }

    auto vertex_shader   = load_shader(desc.stage.vertex_shader);
    auto fragment_shader = load_shader(desc.stage.fragment_shader);

    UniqueVkHandle<VkPipelineLayout, VkDevice> pipeline_layout;
    {
        VkDescriptorSetLayout descriptor_set_layout = m_descriptor_set_layout;

        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount         = 1;
        pipeline_layout_info.pSetLayouts            = &descriptor_set_layout;
        pipeline_layout_info.pushConstantRangeCount = 0;
        pipeline_layout_info.pPushConstantRanges    = nullptr;

        VkPipelineLayout raw_pipeline_layout{};
        verify(vkCreatePipelineLayout(m_device.device, &pipeline_layout_info, nullptr,
                                      &raw_pipeline_layout));
        pipeline_layout = make_unique_vk_handle(raw_pipeline_layout, m_device.device, nullptr,
                                                vkDestroyPipelineLayout);
    }

    // Initialize the non-mutable parts of the pipeline
    *it                    = std::make_unique<RenderPipeline>();
    (*it)->vertex_shader   = std::move(vertex_shader);
    (*it)->fragment_shader = std::move(fragment_shader);
    (*it)->pipeline_layout = std::move(pipeline_layout);

    // Create the dynamic part of the pipeline from the non-mutable parts
    recreate_render_pipeline(**it);

    return it - m_render_pipelines.begin();
}

void Renderer::recreate_render_pipeline(RenderPipeline& pipeline)
{
    // Clear the old resources
    pipeline.framebuffers.clear();
    pipeline.pipeline    = {};
    pipeline.render_pass = {};

    std::array<VkVertexInputBindingDescription, 1> binding_descriptions{};
    binding_descriptions[0].binding   = 0;
    binding_descriptions[0].stride    = sizeof(Mesh::Vertex);
    binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions{};
    attribute_descriptions[0].binding  = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[0].offset   = offsetof(Mesh::Vertex, position);
    attribute_descriptions[1].binding  = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset   = offsetof(Mesh::Vertex, normal);

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount =
        static_cast<std::uint32_t>(binding_descriptions.size());
    vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();
    vertex_input_info.vertexAttributeDescriptionCount =
        static_cast<std::uint32_t>(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x        = 0.0F;
    viewport.y        = 0;
    viewport.width    = static_cast<float>(m_swapchain.extent.width);
    viewport.height   = static_cast<float>(m_swapchain.extent.height);
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapchain.extent;

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports    = &viewport;
    viewport_state.scissorCount  = 1;
    viewport_state.pScissors     = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0F;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0F;
    rasterizer.depthBiasClamp          = 0.0F;
    rasterizer.depthBiasSlopeFactor    = 0.0F;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0F;
    multisampling.pSampleMask           = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable         = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable     = VK_FALSE;
    color_blending.logicOp           = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount   = 1;
    color_blending.pAttachments      = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0F;
    color_blending.blendConstants[1] = 0.0F;
    color_blending.blendConstants[2] = 0.0F;
    color_blending.blendConstants[3] = 0.0F;

    VkAttachmentDescription color_attachment{};
    color_attachment.format         = m_swapchain.image_format;
    color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format         = m_swapchain.depth_format;
    depth_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkPipelineShaderStageCreateInfo vertex_shader_stage_info{};
    vertex_shader_stage_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_stage_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_stage_info.module = pipeline.vertex_shader;
    vertex_shader_stage_info.pName  = "main";

    VkPipelineShaderStageCreateInfo fragment_shader_stage_info{};
    fragment_shader_stage_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_stage_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_stage_info.module = pipeline.fragment_shader;
    fragment_shader_stage_info.pName  = "main";

    UniqueVkHandle<VkRenderPass, VkDevice> render_pass;
    {
        VkSubpassDependency dependency{};
        dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass    = 0;
        dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        const std::array<VkAttachmentDescription, 2> attachments = {color_attachment,
                                                                    depth_attachment};

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<std::uint32_t>(attachments.size());
        renderPassInfo.pAttachments    = attachments.data();
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;

        VkRenderPass raw_render_pass{};
        verify(vkCreateRenderPass(m_device.device, &renderPassInfo, nullptr, &raw_render_pass));
        render_pass =
            make_unique_vk_handle(raw_render_pass, m_device.device, nullptr, vkDestroyRenderPass);
    }

    UniqueVkHandle<VkPipeline, VkDevice> vulkan_pipeline;
    {
        const std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages{
            vertex_shader_stage_info, fragment_shader_stage_info};

        VkPipelineDepthStencilStateCreateInfo depth_stencil{};
        depth_stencil.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthTestEnable  = VK_TRUE;
        depth_stencil.depthWriteEnable = VK_TRUE;
        depth_stencil.depthCompareOp   = VK_COMPARE_OP_LESS;
        depth_stencil.depthBoundsTestEnable = VK_FALSE;
        depth_stencil.stencilTestEnable     = VK_FALSE;

        VkGraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount          = static_cast<std::uint32_t>(shader_stages.size());
        pipeline_info.pStages             = shader_stages.data();
        pipeline_info.pVertexInputState   = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &input_assembly;
        pipeline_info.pViewportState      = &viewport_state;
        pipeline_info.pRasterizationState = &rasterizer;
        pipeline_info.pMultisampleState   = &multisampling;
        pipeline_info.pDepthStencilState  = &depth_stencil;
        pipeline_info.pColorBlendState    = &color_blending;
        pipeline_info.pDynamicState       = nullptr;
        pipeline_info.layout              = pipeline.pipeline_layout;
        pipeline_info.renderPass          = render_pass;
        pipeline_info.subpass             = 0;
        pipeline_info.basePipelineHandle  = VK_NULL_HANDLE;
        pipeline_info.basePipelineIndex   = -1;

        VkPipeline raw_pipeline{};
        verify(vkCreateGraphicsPipelines(m_device.device, VK_NULL_HANDLE, 1, &pipeline_info,
                                         nullptr, &raw_pipeline));
        vulkan_pipeline =
            make_unique_vk_handle(raw_pipeline, m_device.device, nullptr, vkDestroyPipeline);
    }

    std::vector<UniqueVkHandle<VkFramebuffer, VkDevice>> framebuffers;
    framebuffers.reserve(m_swapchain.image_views.size());
    for (const auto& image_view : m_swapchain.image_views) {
        // Reuse the same depth image because only one frame will be rendered at a time
        std::array<VkImageView, 2> attachments = {image_view, m_swapchain.depth_image_view};

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass      = render_pass;
        framebuffer_info.attachmentCount = static_cast<std::uint32_t>(attachments.size());
        framebuffer_info.pAttachments    = attachments.data();
        framebuffer_info.width           = m_swapchain.extent.width;
        framebuffer_info.height          = m_swapchain.extent.height;
        framebuffer_info.layers          = 1;

        VkFramebuffer raw_framebuffer{};
        verify(vkCreateFramebuffer(m_device.device, &framebuffer_info, nullptr, &raw_framebuffer));
        framebuffers.push_back(
            make_unique_vk_handle(raw_framebuffer, m_device.device, nullptr, vkDestroyFramebuffer));
    }

    pipeline.render_pass  = std::move(render_pass);
    pipeline.pipeline     = std::move(vulkan_pipeline);
    pipeline.framebuffers = std::move(framebuffers);
}

void Renderer::destroy_render_pipeline(pipeline_id pipeline_id)
{
    assert(pipeline_id < m_render_pipelines.size());
    assert(m_render_pipelines[pipeline_id] != nullptr);

    // Make sure this pipeline isn't being used
    vkDeviceWaitIdle(m_device.device);

    m_render_pipelines[pipeline_id] = nullptr;
}

} // namespace khepri::renderer::vulkan