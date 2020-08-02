#pragma once

#include <khepri/renderer/vulkan/detail/memory.hpp>

#include <vulkan/vulkan.h>

namespace khepri::renderer::vulkan {

class RenderableMesh
{
public:
    detail::UniqueVkHandle<VkBuffer, VkDevice>       vertex_buffer;
    detail::UniqueVkHandle<VkDeviceMemory, VkDevice> vertex_buffer_memory;
    std::uint32_t                                    vertex_count{};

    detail::UniqueVkHandle<VkBuffer, VkDevice>       index_buffer;
    detail::UniqueVkHandle<VkDeviceMemory, VkDevice> index_buffer_memory;
    std::uint32_t                                    index_count{};
};

} // namespace khepri::renderer::vulkan
