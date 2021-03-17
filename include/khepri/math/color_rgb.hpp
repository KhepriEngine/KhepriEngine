#pragma once

#include "math.hpp"

#include <gsl/gsl-lite.hpp>

#include <cassert>

namespace khepri {

class ColorRGBA;
class ColorSRGB;

/**
 * \brief An RGB color
 *
 * This color is in \a linear space, so mathematical operations have the expected result.
 * However, it must be converted to a \ref ColorSRGB before displaying to a user.
 *
 * This class is similar to \ref Vector3, except it describes the semantics of its contents,
 * and it does not provide geometric operations such as \a length, \a dot, etc.
 *
 * \note This class does \a not clamp results after mathematical operations to the [0,1] range.
 */
#pragma pack(push, 1)
class ColorRGB final
{
public:
    /// The type of the color's components
    using component_type = float;

    /// The red component of the color
    component_type r{};

    /// The green component of the color
    component_type g{};

    /// The blue component of the color
    component_type b{};

    /// Constructs an uninitialized color_rgb
    constexpr ColorRGB() noexcept = default;

    /// Constructs the color_rgb from literals
    constexpr ColorRGB(component_type fr, component_type fg, component_type fb) noexcept
        : r(fr), g(fg), b(fb)
    {}

    /// Constructs the color_rgb from a ColorRGBA by throwing away the Alpha component
    explicit constexpr ColorRGB(const ColorRGBA& c) noexcept;

    /**
     * Constructs a ColorRGB from a ColorSRGB by performing sRGB-to-linear conversion.
     *
     * \note the components of the resulting color are in the domain [0,1].
     */
    explicit constexpr ColorRGB(const ColorSRGB& c) noexcept;

    /// Adds color \a c to the vector
    ColorRGB& operator+=(const ColorRGB& c) noexcept
    {
        r += c.r;
        g += c.g;
        b += c.b;
        return *this;
    }

    /// Subtracts color \a c from the color
    ColorRGB& operator-=(const ColorRGB& c) noexcept
    {
        r -= c.r;
        g -= c.g;
        b -= c.b;
        return *this;
    }

    /// Scales the color by scalar \a s
    ColorRGB& operator*=(component_type s) noexcept
    {
        r *= s;
        g *= s;
        b *= s;
        return *this;
    }

    /// Scales the color with scalar 1 / \a s
    ColorRGB& operator/=(component_type s) noexcept
    {
        r /= s;
        g /= s;
        b /= s;
        return *this;
    }

    /// Indexes the color. 0 is Red, 1 is Green, etc
    [[nodiscard]] const component_type& operator[](int index) const noexcept
    {
        assert(index < 3);
        return gsl::span<const component_type>(&r, 3)[index];
    }

    /// Indexes the color. 0 is Red, 1 is Green, etc
    component_type& operator[](int index) noexcept
    {
        assert(index < 3);
        return gsl::span<component_type>(&r, 3)[index];
    }
};
#pragma pack(pop)

/// Negates color \a c
inline ColorRGB operator-(const ColorRGB& c) noexcept
{
    return ColorRGB(-c.r, -c.g, -c.b);
}

/// Adds color \a c2 to vector \a c1
inline ColorRGB operator+(const ColorRGB& c1, const ColorRGB& c2) noexcept
{
    return ColorRGB(c1.r + c2.r, c1.g + c2.g, c1.b + c2.b);
}

/// Subtracts color \a c2 from color \a c1
inline ColorRGB operator-(const ColorRGB& c1, const ColorRGB& c2) noexcept
{
    return ColorRGB(c1.r - c2.r, c1.g - c2.g, c1.b - c2.b);
}

/// Scales color \a c with scalar \a s
inline ColorRGB operator*(const ColorRGB& c, float s) noexcept
{
    return ColorRGB(c.r * s, c.g * s, c.b * s);
}

/// Scales color \a c with scalar \a s
inline ColorRGB operator*(float s, const ColorRGB& c) noexcept
{
    return ColorRGB(c.r * s, c.g * s, c.b * s);
}

/// Scales vector \a c with scalar 1/\a s
inline ColorRGB operator/(const ColorRGB& c, float s) noexcept
{
    return ColorRGB(c.r / s, c.g / s, c.b / s);
}

/// Modulates color \a c1 with color \a c2
inline ColorRGB operator*(const ColorRGB& c1, const ColorRGB& c2) noexcept
{
    return ColorRGB(c1.r * c2.r, c1.g * c2.g, c1.b * c2.b);
}

/**
 * \brief Clamps each component of a color between two extremes
 *
 * Returns \a min if \a val.{r,g,b} < \a min.
 * Returns \a max if \a val.{r,g,b} > \a max.
 * Otherwise, returns \a val.{r,g,b}.
 *
 * \param[in] col the color to clamp
 * \param[in] min the lower boundary to clamp against
 * \param[in] max the upper boundary to clamp against
 *
 * \return \a val, with each component clamped between \a min and \a max.
 */
constexpr ColorRGB clamp(const ColorRGB& col, ColorRGB::component_type min,
                         ColorRGB::component_type max) noexcept
{
    return {clamp(col.r, min, max), clamp(col.g, min, max), clamp(col.b, min, max)};
}

} // namespace khepri