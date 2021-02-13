#pragma once

#include <cctype>
#include <optional>
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
 * Checks if two strings are equal, ignoring case.
 */
bool case_insensitive_equals(std::string_view s1, std::string_view s2);

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

/**
 * Tokenizes a string
 */
class Tokenizer
{
public:
    /**
     * Constructs the tokenizer.
     *
     * Calling #next repeatedly on a tokenizer returns subsequent substrings such that the
     * substrings are delimited by one or more of the specified delimiters.
     *
     * \param input the input string to tokenize
     * \param delimiters the delimiters to tokenize on (default = all whitespace characters)
     *
     * \note the input strings are not copied, the caller must ensure they remain valid.
     */
    Tokenizer(std::string_view input, std::string_view delimiters = " \t\r\n\v\f");

    /**
     * Returns the next token from the input string, or std::none if there are no more tokens.
     */
    std::optional<std::string_view> next() noexcept;

private:
    std::string_view            m_input;
    std::string_view            m_delimiters;
    std::string_view::size_type m_next;
};

} // namespace khepri