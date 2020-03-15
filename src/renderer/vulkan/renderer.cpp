#include "exceptions.hpp"

#include <khepri/log/log.hpp>
#include <khepri/renderer/vulkan/renderer.hpp>

#include <gsl/gsl-lite.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <optional>
#include <unordered_map>
#include <vector>

namespace khepri::renderer::vulkan {

using detail::make_unique_vk_handle;

struct Renderer::GraphicsDeviceInfo
{
    VkPhysicalDevice                physical_device{};
    std::uint32_t                   graphics_queue_family_index{};
    std::uint32_t                   graphics_queue_count{};
    std::uint32_t                   present_queue_family_index{};
    std::uint32_t                   present_queue_count{};
    std::vector<VkSurfaceFormatKHR> surface_formats{};
    std::vector<VkPresentModeKHR>   present_modes{};
};

namespace {
constexpr log::Logger LOG("vulkan");

void verify(VkResult result)
{
    if (result != VK_SUCCESS) {
        LOG.error("error code " + std::to_string(result));
        throw Error("error code " + std::to_string(result));
    }
}

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
} // namespace

Renderer::Renderer(const char* application_name, SurfaceProvider& surface_provider)
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

    // Enumerate physical device groups and find the physical devices that can render to the
    // obtained surface
    const std::vector<std::string> device_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    const auto graphics_device = find_graphics_device(m_instance, m_surface, device_extensions);
    if (!graphics_device) {
        throw Error("No valid graphics devices found");
    }

    // Create logical device from the found physical device
    {
        // We only need one queue
        const std::array<float, 1> queue_priorities{1.0F};

        VkDeviceQueueCreateInfo queue_info = {};
        queue_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex        = graphics_device->graphics_queue_family_index;
        queue_info.queueCount              = queue_priorities.size();
        queue_info.pQueuePriorities        = queue_priorities.data();

        const auto vk_device_extensions = to_const_char_vector(device_extensions);

        VkDeviceCreateInfo create_info    = {};
        create_info.sType                 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount  = 1;
        create_info.pQueueCreateInfos     = &queue_info;
        create_info.enabledExtensionCount = static_cast<std::uint32_t>(vk_device_extensions.size());
        create_info.ppEnabledExtensionNames = vk_device_extensions.data();

        VkDevice device{};
        verify(vkCreateDevice(graphics_device->physical_device, &create_info, nullptr, &device));
        m_device = UniqueVkInstance<VkDevice>(device, nullptr, vkDestroyDevice);
    }

    // Create the swap chain
    const auto surface_size = surface_provider.get_render_size();
    m_swapchain             = create_swapchain(m_device, *graphics_device, m_surface, surface_size);

    LOG.info("Created {}px x {}px swapchain with {} images", m_swapchain.extent.width,
             m_swapchain.extent.height, m_swapchain.images.size());
}

Renderer::~Renderer() = default;

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

Renderer::SwapchainInfo Renderer::create_swapchain(VkDevice                  device,
                                                   const GraphicsDeviceInfo& device_info,
                                                   VkSurfaceKHR surface, const Size& surface_size)
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    verify(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_info.physical_device, surface,
                                                     &surface_capabilities));

    const auto surface_format = pick_swapchain_surface_format(device_info.surface_formats);
    const auto present_mode   = pick_swapchain_present_mode(device_info.present_modes);

    SwapchainInfo SwapchainInfo;

    {
        // Try to at least double buffer
        uint32_t image_count = surface_capabilities.minImageCount + 1;
        if (surface_capabilities.maxImageCount > 0) {
            image_count = std::min(image_count, surface_capabilities.maxImageCount);
        }

        VkSwapchainCreateInfoKHR create_info = {};
        create_info.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface                  = surface;
        create_info.minImageCount            = image_count;
        create_info.imageFormat              = surface_format.format;
        create_info.imageColorSpace          = surface_format.colorSpace;
        create_info.imageExtent              = pick_swapchain_extent(
            surface_capabilities, {static_cast<std::uint32_t>(surface_size.width),
                                   static_cast<std::uint32_t>(surface_size.height)});
        create_info.imageArrayLayers = 1; // Non-stereoscopic
        create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        create_info.preTransform     = surface_capabilities.currentTransform;
        create_info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode      = present_mode;
        create_info.clipped          = VK_TRUE;

        const std::array<std::uint32_t, 2> queue_family_indices{
            device_info.graphics_queue_family_index, device_info.present_queue_family_index};

        if (device_info.graphics_queue_family_index != device_info.present_queue_family_index) {
            // We're sharing the swap chain
            create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = queue_family_indices.size();
            create_info.pQueueFamilyIndices   = queue_family_indices.data();
        } else {
            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        VkSwapchainKHR swapchain{};
        verify(vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain));
        SwapchainInfo.handle =
            make_unique_vk_handle(swapchain, device, nullptr, vkDestroySwapchainKHR);
        SwapchainInfo.extent = create_info.imageExtent;
        SwapchainInfo.format = create_info.imageFormat;
    }

    SwapchainInfo.images = get_swapchain_images(device, SwapchainInfo.handle);

    SwapchainInfo.image_views.resize(SwapchainInfo.images.size());
    for (std::size_t i = 0; i < SwapchainInfo.images.size(); i++) {
        VkImageViewCreateInfo create_info           = {};
        create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image                           = SwapchainInfo.images[i];
        create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format                          = SwapchainInfo.format;
        create_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel   = 0;
        create_info.subresourceRange.levelCount     = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount     = 1;
        VkImageView image_view{};
        verify(vkCreateImageView(device, &create_info, nullptr, &image_view));
        SwapchainInfo.image_views[i] =
            make_unique_vk_handle(image_view, device, nullptr, vkDestroyImageView);
    }

    return SwapchainInfo;
}

} // namespace khepri::renderer::vulkan
