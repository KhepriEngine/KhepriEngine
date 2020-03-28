#pragma once

#include <cmath>

namespace khepri {

/// \brief A discrete x,y coordinate pair
class Point final
{
public:
    /// The type of the point's components
    using component_type = long;

    /// The x coordinate of this point
    component_type x;

    /// The y coordinate of this point
    component_type y;

    /// Constructs an uninitialized point
    Point() noexcept = default;

    /// Constructs the point \a x and \a y.
    Point(component_type x, component_type y) noexcept : x(x), y(y) {}
};

} // namespace khepri
