#pragma once

#include <type_traits>

namespace khepri {

static constexpr double PI = 3.1415926535897932384626433832795;

/**
 * \brief Clamps a value between two extremes
 *
 * Returns \a min if \a val < \a min.
 * Returns \a max if \a val > \a max.
 * Otherwise, returns \a val.
 *
 * \param[in] val the value to clamp
 * \param[in] min the lower boundary to clamp against
 * \param[in] max the upper boundary to clamp against
 *
 * \return \a val clamped between \a min and \a max.
 */
template <typename T>
constexpr T clamp(const T& val, const T& min, const T& max) noexcept
{
    static_assert(std::is_floating_point_v<T>);
    return (val <= min) ? min : (val >= max) ? max : val;
}

/**
 * \brief Clamps a value between 0 and 1
 *
 * Returns 0 if \a val < 0.
 * Returns 1 if \a val > 1.
 * Otherwise, returns \a val.
 *
 * \param[in] val the value to clamp
 *
 * \return \a val clamped between 0 and 1.
 */
template <typename T>
constexpr T saturate(const T& val) noexcept
{
    static_assert(std::is_floating_point_v<T>);
    return clamp(val, T{0}, T{1});
}

/// Converts degrees to radians
template <typename T>
constexpr T to_radians(const T& degrees) noexcept
{
    constexpr auto DEGREES_PER_PI_RADIANS = 180;
    static_assert(std::is_floating_point_v<T>);
    return degrees / DEGREES_PER_PI_RADIANS * T{PI};
}

/// Converts radians to degrees
template <typename T>
constexpr T to_degrees(const T& radians) noexcept
{
    constexpr auto DEGREES_PER_PI_RADIANS = 180;
    static_assert(std::is_floating_point_v<T>);
    return radians / T{PI} * DEGREES_PER_PI_RADIANS;
}

} // namespace khepri
