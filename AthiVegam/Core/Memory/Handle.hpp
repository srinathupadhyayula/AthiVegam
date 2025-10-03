#pragma once

#include "Core/Types.hpp"
#include <limits>

/// @file Handle.hpp
/// @brief Versioned handle system for stable references

namespace Engine {

/// @brief 64-bit versioned handle (32-bit index + 32-bit version)
/// @tparam T Type tag for type safety
template<typename T>
class Handle {
public:
    /// @brief Create an invalid handle
    constexpr Handle() : _value(InvalidValue) {}

    /// @brief Create a handle from index and version
    constexpr Handle(u32 index, u32 version)
        : _value((static_cast<u64>(version) << 32) | index) {}

    /// @brief Get the index component
    constexpr u32 Index() const { return static_cast<u32>(_value & 0xFFFFFFFF); }

    /// @brief Get the version component
    constexpr u32 Version() const { return static_cast<u32>(_value >> 32); }

    /// @brief Check if handle is valid
    constexpr bool IsValid() const { return _value != InvalidValue; }

    /// @brief Explicit conversion to bool
    constexpr explicit operator bool() const { return IsValid(); }

    /// @brief Get raw 64-bit value
    constexpr u64 Value() const { return _value; }

    /// @brief Equality comparison
    constexpr bool operator==(const Handle& other) const { return _value == other._value; }
    constexpr bool operator!=(const Handle& other) const { return _value != other._value; }

    /// @brief Ordering for use in containers
    constexpr bool operator<(const Handle& other) const { return _value < other._value; }

    /// @brief Invalid handle constant
    static constexpr u64 InvalidValue = std::numeric_limits<u64>::max();

private:
    u64 _value;
};

/// @brief Handle hash for use in unordered containers
template<typename T>
struct HandleHash {
    usize operator()(const Handle<T>& handle) const {
        return static_cast<usize>(handle.Value());
    }
};

} // namespace Engine

