#include <khepri/utility/string.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace khepri {

std::string basename(std::string_view str)
{
    return std::filesystem::path(str).filename().replace_extension("").string();
}

std::string uppercase(std::string_view str)
{
    std::string result(str.size(), '\0');
    std::transform(str.begin(), str.end(), result.begin(),
                   [](unsigned char ch) { return toupper(ch); });
    return result;
}

Tokenizer::Tokenizer(std::string_view input, std::string_view delimiters)
    : m_input(input), m_delimiters(delimiters), m_next(m_input.find_first_not_of(m_delimiters))
{}

std::optional<std::string_view> Tokenizer::next() noexcept
{
    if (m_next == std::string_view::npos) {
        return {};
    }

    const auto start = m_next;
    const auto end   = m_input.find_first_of(m_delimiters, start);
    const auto count = ((end != std::string_view::npos) ? end : m_input.size()) - start;

    m_next = m_input.find_first_not_of(m_delimiters, end);
    return m_input.substr(start, count);
}

} // namespace khepri