#include "Logger.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <filesystem>
#include <vector>

namespace Engine {

// Static member initialization
std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;
bool Logger::s_initialized = false;

// Helper to convert LogLevel to spdlog level
static spdlog::level::level_enum ToSpdlogLevel(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:    return spdlog::level::trace;
        case LogLevel::Debug:    return spdlog::level::debug;
        case LogLevel::Info:     return spdlog::level::info;
        case LogLevel::Warn:     return spdlog::level::warn;
        case LogLevel::Error:    return spdlog::level::err;
        case LogLevel::Critical: return spdlog::level::critical;
        case LogLevel::Off:      return spdlog::level::off;
        default:                 return spdlog::level::info;
    }
}

void Logger::Initialize(const LoggerConfig& config) {
    if (s_initialized) {
        return; // Already initialized
    }
    
    std::vector<spdlog::sink_ptr> sinks;
    
    // Console sink
    if (config.enableConsole) {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(ToSpdlogLevel(config.consoleLevel));
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        sinks.push_back(console_sink);
    }
    
    // File sink
    if (config.enableFile) {
        // Create logs directory if it doesn't exist
        std::filesystem::path logPath(config.logFilePath);
        std::filesystem::path logDir = logPath.parent_path();
        
        if (!logDir.empty() && !std::filesystem::exists(logDir)) {
            std::filesystem::create_directories(logDir);
        }
        
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            std::string(config.logFilePath),
            config.maxFileSize,
            config.maxFiles
        );
        file_sink->set_level(ToSpdlogLevel(config.fileLevel));
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
        sinks.push_back(file_sink);
    }
    
    // Create logger with sinks
    s_logger = std::make_shared<spdlog::logger>("AthiVegam", sinks.begin(), sinks.end());
    s_logger->set_level(spdlog::level::trace); // Allow all levels, sinks will filter
    s_logger->flush_on(spdlog::level::err); // Auto-flush on errors
    
    // Register as default logger
    spdlog::set_default_logger(s_logger);
    
    s_initialized = true;
    
    // Log initialization
    Info("Logger initialized");
    Debug("Console level: {}", static_cast<int>(config.consoleLevel));
    Debug("File level: {}", static_cast<int>(config.fileLevel));
    if (config.enableFile) {
        Debug("Log file: {}", config.logFilePath);
    }
}

void Logger::Shutdown() {
    if (!s_initialized) {
        return;
    }
    
    Info("Logger shutting down");
    Flush();
    
    s_logger.reset();
    spdlog::shutdown();
    s_initialized = false;
}

bool Logger::IsInitialized() {
    return s_initialized;
}

void Logger::SetConsoleLevel(LogLevel level) {
    if (s_initialized && s_logger) {
        // Find console sink and update its level
        auto sinks = s_logger->sinks();
        for (auto& sink : sinks) {
            // Console sink is typically the first one
            auto console_sink = std::dynamic_pointer_cast<spdlog::sinks::stdout_color_sink_mt>(sink);
            if (console_sink) {
                console_sink->set_level(ToSpdlogLevel(level));
                Debug("Console log level changed to {}", static_cast<int>(level));
                break;
            }
        }
    }
}

void Logger::SetFileLevel(LogLevel level) {
    if (s_initialized && s_logger) {
        // Find file sink and update its level
        auto sinks = s_logger->sinks();
        for (auto& sink : sinks) {
            auto file_sink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_mt>(sink);
            if (file_sink) {
                file_sink->set_level(ToSpdlogLevel(level));
                Debug("File log level changed to {}", static_cast<int>(level));
                break;
            }
        }
    }
}

void Logger::Flush() {
    if (s_initialized && s_logger) {
        s_logger->flush();
    }
}

} // namespace Engine

