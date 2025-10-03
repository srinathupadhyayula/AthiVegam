#pragma once

#include "Logger.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

namespace Engine {

template<typename... Args>
void Logger::Trace(std::string_view fmt, Args&&... args) {
    if (s_initialized && s_logger) {
        s_logger->trace(fmt::runtime(fmt), std::forward<Args>(args)...);
    }
}

template<typename... Args>
void Logger::Debug(std::string_view fmt, Args&&... args) {
    if (s_initialized && s_logger) {
        s_logger->debug(fmt::runtime(fmt), std::forward<Args>(args)...);
    }
}

template<typename... Args>
void Logger::Info(std::string_view fmt, Args&&... args) {
    if (s_initialized && s_logger) {
        s_logger->info(fmt::runtime(fmt), std::forward<Args>(args)...);
    }
}

template<typename... Args>
void Logger::Warn(std::string_view fmt, Args&&... args) {
    if (s_initialized && s_logger) {
        s_logger->warn(fmt::runtime(fmt), std::forward<Args>(args)...);
    }
}

template<typename... Args>
void Logger::Error(std::string_view fmt, Args&&... args) {
    if (s_initialized && s_logger) {
        s_logger->error(fmt::runtime(fmt), std::forward<Args>(args)...);
    }
}

template<typename... Args>
void Logger::Critical(std::string_view fmt, Args&&... args) {
    if (s_initialized && s_logger) {
        s_logger->critical(fmt::runtime(fmt), std::forward<Args>(args)...);
    }
}

} // namespace Engine

