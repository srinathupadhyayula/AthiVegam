#pragma once

#include "Core/Types.hpp"
#include <span>

/// @file Span.hpp
/// @brief Non-owning view over contiguous memory

namespace Engine {

/// @brief Non-owning view over contiguous sequence of T
/// @tparam T Element type
template<typename T>
using Span = std::span<T>;

/// @brief Create a span from a pointer and size
template<typename T>
constexpr Span<T> make_span(T* data, usize size) {
    return Span<T>(data, size);
}

/// @brief Create a span from a container
template<typename Container>
constexpr auto make_span(Container& container) {
    return Span(container.data(), container.size());
}

/// @brief Create a const span from a container
template<typename Container>
constexpr auto make_span(const Container& container) {
    return Span(container.data(), container.size());
}

/// @brief Reinterpret span as bytes
template<typename T>
constexpr Span<const byte> as_bytes(Span<T> span) {
    return std::as_bytes(span);
}

/// @brief Reinterpret span as writable bytes
template<typename T>
constexpr Span<byte> as_writable_bytes(Span<T> span) {
    return std::as_writable_bytes(span);
}

} // namespace Engine

