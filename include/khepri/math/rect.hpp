#pragma once

namespace khepri {

/**
 * \brief Represents a discrete, two-dimensional rectangle
 */
struct Rect
{
    /// The x coordinate of the top-left point of the rectangle
    long x;

    /// The y coordinate of the top-left point of the rectangle
    long y;

    /// The width of the rectangle
    unsigned long width;

    /// The height of the rectangle
    unsigned long height;
};

} // namespace khepri