#pragma once

#include <khepri/exceptions.hpp>

namespace khepri::renderer {

/**
 * Base class for errors in the renderer
 */
class Error : public khepri::Error
{
public:
    using khepri::Error::Error;
};

} // namespace khepri::renderer
