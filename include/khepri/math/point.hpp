#pragma once

#include <cmath>

namespace khepri {

/// \brief A discrete x,y coordinate pair
class Point final
{
public:
    /// The type of the point's components
    using ComponentType = long;

    /// The x coordinate of this point
    ComponentType x;

    /// The y coordinate of this point
    ComponentType y;

    /// Constructs an uninitialized point
    Point() noexcept = default;

    /// Constructs the point \a x and \a y.
    Point(ComponentType x, ComponentType y) noexcept : x(x), y(y) {}
};

inline constexpr bool operator==(const Point& p1, const Point& p2) noexcept
{
    return p1.x == p2.x && p1.y == p2.y;
}

inline constexpr bool operator!=(const Point& p1, const Point& p2) noexcept
{
    return !(p1 == p2);
}

} // namespace khepri
