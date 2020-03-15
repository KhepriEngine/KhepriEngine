#ifndef KHEPRI_MATH_SIZE_HPP
#define KHEPRI_MATH_SIZE_HPP

namespace khepri {

/**
 * \brief Represents a discrete, two-dimensional size
 */
struct Size
{
    /// The width of the size
    unsigned long width;

    /// The height of the size
    unsigned long height;
};

}; // namespace khepri

#endif