#pragma once

#include "Core/Types.hpp"
#include <string_view>
#include <string>
#include <memory>

/// @file Logger.hpp
/// @brief Logging system wrapping spdlog with severity levels and multiple sinks

namespace spdlog {
    class logger;
}

namespace Engine {

/// @brief Log severity levels
enum class LogLevel : u8 {
    Trace = 0,    ///< Detailed trace information
    Debug = 1,    ///< Debug information
    Info = 2,     ///< Informational messages
    Warn = 3,     ///< Warning messages
    Error = 4,    ///< Error messages
    Critical = 5, ///< Critical errors
    Off = 6       ///< Logging disabled
};

/// @brief Logger configuration
struct LoggerConfig {
    LogLevel consoleLevel = LogLevel::Info;     ///< Console output level
    LogLevel fileLevel = LogLevel::Debug;       ///< File output level
    std::string logFilePath = "logs/athivegam.log"; ///< Log file path
    usize maxFileSize = 1024 * 1024 * 5;        ///< Max file size (5MB)
    usize maxFiles = 3;                          ///< Max rotating files
    bool enableConsole = true;                   ///< Enable console output
    bool enableFile = true;                      ///< Enable file output
    bool enableColor = true;                     ///< Enable colored console output
};

/// @brief Logging system
/// @details Wraps spdlog with a clean interface for engine-wide logging.
///          Supports console and file sinks with independent severity levels.
/// @example
/// @code
/// Logger::Initialize();
/// Logger::Info("Engine initialized");
/// Logger::Warn("Low memory: {} MB", availableMemory);
/// Logger::Shutdown();
/// @endcode
class Logger {
public:
    /// @brief Initialize logging system
    /// @param config Logger configuration
    static void Initialize(const LoggerConfig& config = LoggerConfig{});
    
    /// @brief Shutdown logging system
    static void Shutdown();
    
    /// @brief Check if logger is initialized
    /// @return True if initialized
    static bool IsInitialized();
    
    /// @brief Set console log level
    /// @param level New log level
    static void SetConsoleLevel(LogLevel level);
    
    /// @brief Set file log level
    /// @param level New log level
    static void SetFileLevel(LogLevel level);
    
    /// @brief Log trace message
    /// @tparam Args Argument types
    /// @param fmt Format string
    /// @param args Format arguments
    template<typename... Args>
    static void Trace(std::string_view fmt, Args&&... args);
    
    /// @brief Log debug message
    /// @tparam Args Argument types
    /// @param fmt Format string
    /// @param args Format arguments
    template<typename... Args>
    static void Debug(std::string_view fmt, Args&&... args);
    
    /// @brief Log info message
    /// @tparam Args Argument types
    /// @param fmt Format string
    /// @param args Format arguments
    template<typename... Args>
    static void Info(std::string_view fmt, Args&&... args);
    
    /// @brief Log warning message
    /// @tparam Args Argument types
    /// @param fmt Format string
    /// @param args Format arguments
    template<typename... Args>
    static void Warn(std::string_view fmt, Args&&... args);
    
    /// @brief Log error message
    /// @tparam Args Argument types
    /// @param fmt Format string
    /// @param args Format arguments
    template<typename... Args>
    static void Error(std::string_view fmt, Args&&... args);
    
    /// @brief Log critical message
    /// @tparam Args Argument types
    /// @param fmt Format string
    /// @param args Format arguments
    template<typename... Args>
    static void Critical(std::string_view fmt, Args&&... args);
    
    /// @brief Flush all sinks
    static void Flush();

private:
    static std::shared_ptr<spdlog::logger> s_logger;
    static bool s_initialized;
};

} // namespace Engine

// Logging macros for convenience
#define LOG_TRACE(...)    ::Engine::Logger::Trace(__VA_ARGS__)
#define LOG_DEBUG(...)    ::Engine::Logger::Debug(__VA_ARGS__)
#define LOG_INFO(...)     ::Engine::Logger::Info(__VA_ARGS__)
#define LOG_WARN(...)     ::Engine::Logger::Warn(__VA_ARGS__)
#define LOG_ERROR(...)    ::Engine::Logger::Error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::Engine::Logger::Critical(__VA_ARGS__)

// Include template implementations
#include "Logger.inl"

