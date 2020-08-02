#include "exceptions.hpp"
#include "logging.hpp"
#include "render_pipeline.hpp"
#include "renderable_mesh.hpp"

#include <khepri/renderer/camera.hpp>
#include <khepri/renderer/vulkan/renderer.hpp>

#include <gsl/gsl-lite.hpp>

#include <cassert>
#include <chrono>
#include <thread>

namespace khepri::renderer::vulkan {
namespace {
/**
 * Khepri's perspective matrix transforms x,y,z so that after the w divide they end up in the range
 * [-1,1], [-1,1] and [0,1], respectively. Vulkan's Y axis goes down on the screen, so to get Y
 * pointing 'up' on the screen, we need to invert it.
 *
 * Note that this inverts the front face of triangles, so culling may be affected.
 */
const Matrix VULKAN_PERSPECTIVE_CORRECTION{1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
} // namespace

void Renderer::render_meshes(pipeline_id                             pipeline_id,
                             gsl::span<const RenderableMeshInstance> meshes, const Camera& camera)
{
    assert(pipeline_id < m_render_pipelines.size());

    const auto& pipeline = m_render_pipelines[pipeline_id];

    uint32_t image_index = 0;
    VkResult result =
        vkAcquireNextImageKHR(m_device.device, m_swapchain.handle, UINT64_MAX,
                              m_image_available_semaphore, VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreate_swapchain();
        return;
    }

    if (result != VK_SUCCESS) {
        throw Error("failed to acquire swap chain image!");
    }

    VkCommandBuffer command_buffer = m_swapchain.commandbuffers[image_index];

    gsl::finally([this] { vkDeviceWaitIdle(m_device.device); });

    // Record command buffer
    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags            = 0;
        begin_info.pInheritanceInfo = nullptr;
        verify(vkBeginCommandBuffer(command_buffer, &begin_info));

        std::array<VkClearValue, 2> clear_values{};
        clear_values[0].color = {0.0F, 0.0F, 0.0F, 1.0F}; // NOLINT -- VkClearValue is a union
        clear_values[1].depthStencil = {1.0F, 0};         // NOLINT -- VkClearValue is a union

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass        = pipeline->render_pass;
        render_pass_info.framebuffer       = pipeline->framebuffers[image_index];
        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = m_swapchain.extent;
        render_pass_info.clearValueCount   = static_cast<std::uint32_t>(clear_values.size());
        render_pass_info.pClearValues      = clear_values.data();

        vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline->pipeline_layout, 0, 1,
                                &m_swapchain.descriptor_sets[image_index], 0, nullptr);

        for (const auto& mesh : meshes) {
            assert(mesh.mesh_id < m_renderable_meshes.size());
            const auto& renderable_mesh = m_renderable_meshes[mesh.mesh_id];

            std::array<VkBuffer, 1>     vertex_buffers{renderable_mesh->vertex_buffer};
            std::array<VkDeviceSize, 1> offsets{0};
            vkCmdBindVertexBuffers(command_buffer, 0,
                                   static_cast<std::uint32_t>(vertex_buffers.size()),
                                   vertex_buffers.data(), offsets.data());
            vkCmdBindIndexBuffer(command_buffer, renderable_mesh->index_buffer, 0,
                                 VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(command_buffer, renderable_mesh->index_count, 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(command_buffer);
        verify(vkEndCommandBuffer(command_buffer));
    }

    // Update globals
    {
        PipelineGlobals* globals{};
        void**           pGlobals = reinterpret_cast<void**>(&globals); // NOLINT -- have to cast
        vkMapMemory(m_device.device, m_swapchain.uniform_buffers_memory[image_index], 0,
                    sizeof(PipelineGlobals), 0, pGlobals);
        globals->view_proj = camera.matrices().view_proj * VULKAN_PERSPECTIVE_CORRECTION;
        vkUnmapMemory(m_device.device, m_swapchain.uniform_buffers_memory[image_index]);
    }

    std::array<VkPipelineStageFlags, 1> wait_stages{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    std::array<VkSemaphore, 1>          wait_semaphores{m_image_available_semaphore};
    std::array<VkSemaphore, 1>          signal_semaphores{m_render_finished_semaphore};
    static_assert(wait_stages.size() == wait_semaphores.size());

    VkSubmitInfo submitInfo{};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount   = static_cast<std::uint32_t>(wait_semaphores.size());
    submitInfo.pWaitSemaphores      = wait_semaphores.data();
    submitInfo.pWaitDstStageMask    = wait_stages.data();
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &command_buffer;
    submitInfo.signalSemaphoreCount = static_cast<std::uint32_t>(signal_semaphores.size());
    submitInfo.pSignalSemaphores    = signal_semaphores.data();

    if (vkQueueSubmit(m_device.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw Error("failed to submit draw command buffer!");
    }

    std::array<VkSwapchainKHR, 1> swapchains{m_swapchain.handle};
    VkPresentInfoKHR              presentInfo{};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = static_cast<std::uint32_t>(signal_semaphores.size());
    presentInfo.pWaitSemaphores    = signal_semaphores.data();
    presentInfo.swapchainCount     = static_cast<std::uint32_t>(swapchains.size());
    presentInfo.pSwapchains        = swapchains.data();
    presentInfo.pImageIndices      = &image_index;
    presentInfo.pResults           = nullptr;
    vkQueuePresentKHR(m_device.present_queue, &presentInfo);
}

} // namespace khepri::renderer::vulkan
