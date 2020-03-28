#include <khepri/version.hpp>

#include <gtest/gtest.h>

TEST(KhepriVersion, VersionNotEmpty)
{
    const auto& version = khepri::version();
    EXPECT_FALSE(khepri::to_string(version).empty());
    EXPECT_NE("", version.build_commit);
}