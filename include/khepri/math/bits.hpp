#pragma once

#include <cstdint>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace khepri {

/// Counts the number of bits in a 16-bit integer
inline unsigned int bitcount(std::uint16_t value)
{
#if defined(_MSC_VER)
    return static_cast<unsigned int>(__popcnt16(value));
#elif defined(__GNUC__) || defined(__clang__)
    return static_cast<unsigned int>(__builtin_popcount(value));
#else
#error "the platform does not support bitcount()"
#endif
}

/// Counts the number of bits in a 32-bit integer
inline unsigned int bitcount(std::uint32_t value)
{
#if defined(_MSC_VER)
    return static_cast<unsigned int>(__popcnt(value));
#elif defined(__GNUC__) || defined(__clang__)
    return static_cast<unsigned int>(__builtin_popcountl(value));
#else
#error "the platform does not support bitcount()"
#endif
}

/// Counts the number of bits in a 64-bit integer
inline unsigned int bitcount(std::uint64_t value)
{
#if defined(_MSC_VER)
    return static_cast<unsigned int>(__popcnt64(value));
#elif defined(__GNUC__) || defined(__clang__)
    return static_cast<unsigned int>(__builtin_popcountll(value));
#else
#error "the platform does not support bitcount()"
#endif
}

} // namespace khepri