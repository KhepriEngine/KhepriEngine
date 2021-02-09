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

} // namespace khepri