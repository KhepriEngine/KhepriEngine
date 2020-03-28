#pragma once

#include <khepri/exceptions.hpp>

namespace khepri::io {

/// Base class for all IO-related errors
class Error : public khepri::Error
{
public:
    using khepri::Error::Error;
};

} // namespace khepri::io
