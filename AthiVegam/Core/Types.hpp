#pragma once

#include <cstdint>
#include <cstddef>

/// @file Types.hpp
/// @brief Fundamental type definitions for the AthiVegam engine

namespace Engine {

// Integer types
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using usize = size_t;
using isize = ptrdiff_t;

// Floating point types
using f32 = float;
using f64 = double;

// Byte type
using byte = uint8_t;

// Constants
inline constexpr usize KiB = 1024;
inline constexpr usize MiB = 1024 * KiB;
inline constexpr usize GiB = 1024 * MiB;

inline constexpr usize CacheLineSize = 64;

} // namespace Engine

