#pragma once

#include <khepri/math/matrix.hpp>
#include <khepri/renderer/vulkan/detail/memory.hpp>

#include <vulkan/vulkan.h>

#include <vector>

namespace khepri::renderer::vulkan {

struct PipelineGlobals
{
    khepri::Matrix view_proj;
};

class RenderPipeline
{
public:
    detail::UniqueVkHandle<VkShaderModule, VkDevice>             vertex_shader;
    detail::UniqueVkHandle<VkShaderModule, VkDevice>             fragment_shader;
    detail::UniqueVkHandle<VkPipelineLayout, VkDevice>           pipeline_layout;
    detail::UniqueVkHandle<VkRenderPass, VkDevice>               render_pass;
    detail::UniqueVkHandle<VkPipeline, VkDevice>                 pipeline;
    std::vector<detail::UniqueVkHandle<VkFramebuffer, VkDevice>> framebuffers;
};

} // namespace khepri::renderer::vulkan
