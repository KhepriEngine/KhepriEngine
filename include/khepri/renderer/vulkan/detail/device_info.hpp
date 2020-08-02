#pragma once

#include "memory.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace khepri::renderer::vulkan::detail {

struct GraphicsDeviceInfo
{
    VkPhysicalDevice                physical_device{};
    std::uint32_t                   graphics_queue_family_index{};
    std::uint32_t                   graphics_queue_count{};
    std::uint32_t                   present_queue_family_index{};
    std::uint32_t                   present_queue_count{};
    std::vector<VkSurfaceFormatKHR> surface_formats{};
    std::vector<VkPresentModeKHR>   present_modes{};
};

struct DeviceInfo
{
    GraphicsDeviceInfo               graphics_device{};
    UniqueVkInstance<VkDevice>       device{};
    VkPhysicalDeviceMemoryProperties mem_properties{};
    VkQueue                          graphics_queue{};
    VkQueue                          present_queue{};
};

} // namespace khepri::renderer::vulkan::detail
