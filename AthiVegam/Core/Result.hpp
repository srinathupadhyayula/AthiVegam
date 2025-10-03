#pragma once

#include <expected>
#include <string>
#include <system_error>

/// @file Result.hpp
/// @brief Result type for error handling

namespace Engine {

/// @brief Error information
struct Error {
    std::error_code code;
    std::string message;
    
    Error() = default;
    Error(std::error_code ec, std::string msg = "")
        : code(ec), message(std::move(msg)) {}
    
    explicit Error(std::string msg)
        : code(std::make_error_code(std::errc::invalid_argument))
        , message(std::move(msg)) {}
};

/// @brief Result type that can hold either a value or an error
/// @tparam T Value type
template<typename T>
using Result = std::expected<T, Error>;

/// @brief Create a successful result
template<typename T>
Result<T> Ok(T&& value) {
    return Result<T>(std::forward<T>(value));
}

/// @brief Create a successful void result
inline std::expected<void, Error> Ok() {
    return std::expected<void, Error>();
}

/// @brief Create an error result
template<typename T>
Result<T> Err(Error error) {
    return Result<T>(std::unexpected(std::move(error)));
}

/// @brief Create an error result with message
template<typename T>
Result<T> Err(std::string message) {
    return Result<T>(std::unexpected(Error(std::move(message))));
}

} // namespace Engine

