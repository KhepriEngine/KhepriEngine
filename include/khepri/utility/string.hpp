#pragma once

#include <cctype>
#include <string>
#include <string_view>
#include <type_traits>

namespace khepri {

/**
 * Returns the base name of a path-like string.
 * The base name is the path's filename, without extension.
 */
std::string basename(std::string_view str);

/**
 * Returns the uppercase version of a string.
 */
std::string uppercase(std::string_view str);

/**
 * Less-than comparator for case-insensitive comparisons on string-like objects.
 *
 * This comparator is a transparent comparator; it can be used instead of @c std::less<> in e.g.
 * @c std::map to avoid the requirement that the key and lookup types are the same.
 */
class CaseInsensitiveLess
{
private:
    struct NoCaseCompare
    {
        bool operator()(unsigned char c1, unsigned char c2) const
        {
            return std::tolower(c1) < std::tolower(c2);
        }
    };

public:
    /// Checks if the first string-like argument is lexicographically less-than the second
    /// string-like argument
    template <typename T, typename U>
    bool operator()(T&& t, U&& u) const
    {
        return std::lexicographical_compare(std::begin(t), std::end(t), std::begin(u), std::end(u),
                                            NoCaseCompare());
    }

    /// Marks the comparator as a transparent comparator
    using is_transparent = std::bool_constant<true>;
};

} // namespace khepri