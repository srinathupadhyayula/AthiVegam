#pragma once

#include "Core/Types.hpp"
#include "Core/Span.hpp"
#include <variant>
#include <string>
#include <vector>
#include <concepts>
#include <type_traits>

namespace Engine::Comm {

/// @brief Concept for valid payload types
/// @details Payloads must be trivially copyable POD types for efficient messaging
template<typename T>
concept PayloadConcept = std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

/// @brief Type-safe message payload using std::variant
/// @details Supports common POD types for efficient message passing.
///          Uses std::variant for compile-time type safety.
/// @example
/// @code
/// Payload payload = 42;
/// if (auto* value = payload.Get<int>()) {
///     // Use *value
/// }
/// @endcode
class Payload
{
public:
    /// @brief Default constructor (empty payload)
    Payload() = default;
    
    /// @brief Construct from int
    Payload(int value) : _data(value) {}
    
    /// @brief Construct from float
    Payload(float value) : _data(value) {}
    
    /// @brief Construct from double
    Payload(double value) : _data(value) {}
    
    /// @brief Construct from bool
    Payload(bool value) : _data(value) {}
    
    /// @brief Construct from u32
    Payload(u32 value) : _data(value) {}
    
    /// @brief Construct from u64
    Payload(u64 value) : _data(value) {}
    
    /// @brief Construct from string view (copies to std::string)
    Payload(std::string_view value) : _data(std::string(value)) {}
    
    /// @brief Construct from C string (copies to std::string)
    Payload(const char* value) : _data(std::string(value)) {}
    
    /// @brief Construct from std::string
    Payload(const std::string& value) : _data(value) {}
    
    /// @brief Construct from std::string (move)
    Payload(std::string&& value) : _data(std::move(value)) {}
    
    /// @brief Get payload as specific type
    /// @tparam T Type to retrieve
    /// @return Pointer to value if type matches, nullptr otherwise
    template<typename T>
    const T* Get() const
    {
        return std::get_if<T>(&_data);
    }
    
    /// @brief Get payload as specific type (mutable)
    /// @tparam T Type to retrieve
    /// @return Pointer to value if type matches, nullptr otherwise
    template<typename T>
    T* Get()
    {
        return std::get_if<T>(&_data);
    }
    
    /// @brief Check if payload holds specific type
    /// @tparam T Type to check
    /// @return True if payload holds type T
    template<typename T>
    bool Is() const
    {
        return std::holds_alternative<T>(_data);
    }
    
    /// @brief Check if payload is empty
    /// @return True if payload holds std::monostate (empty)
    bool IsEmpty() const
    {
        return std::holds_alternative<std::monostate>(_data);
    }
    
    /// @brief Get type index
    /// @return Index of currently held type
    size_t GetTypeIndex() const
    {
        return _data.index();
    }

private:
    /// @brief Variant holding supported payload types
    std::variant<
        std::monostate,  // Empty
        int,
        float,
        double,
        bool,
        u32,
        u64,
        std::string
    > _data;
};

} // namespace Engine::Comm

