#include <khepri/version_info.hpp>

namespace khepri {

namespace {
const ::khepri::VersionInfo BUILD_VERSION_INFO{KHEPRI_VERSION_MAJOR, KHEPRI_VERSION_MINOR,
                                               KHEPRI_VERSION_PATCH, KHEPRI_VERSION_STRING,
                                               KHEPRI_VERSION_CLEAN, KHEPRI_VERSION_COMMIT};
}

const ::khepri::VersionInfo& version() noexcept
{
    return BUILD_VERSION_INFO;
}

} // namespace khepri
