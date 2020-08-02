#pragma once

#include "exceptions.hpp"

#include <khepri/log/log.hpp>

#include <vulkan/vulkan.h>

namespace khepri::renderer::vulkan {

constexpr log::Logger LOG("vulkan");

inline void verify(VkResult result)
{
    if (result != VK_SUCCESS) {
        LOG.error("error code " + std::to_string(result));
        throw Error("error code " + std::to_string(result));
    }
}

} // namespace khepri::renderer::vulkan
