#pragma once

#include <khepri/renderer/exceptions.hpp>

namespace khepri::renderer::vulkan {

class Error : public khepri::renderer::Error
{
public:
    using khepri::renderer::Error::Error;
};

} // namespace khepri::renderer::vulkan
