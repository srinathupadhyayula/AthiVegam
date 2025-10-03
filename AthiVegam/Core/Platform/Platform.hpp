#pragma once

#include "Core/Types.hpp"

/// @file Platform.hpp
/// @brief Platform abstraction layer for Windows-specific functionality

namespace Engine::Platform {

/// @brief Initialize platform-specific systems
/// @return true on success, false on failure
bool Initialize();

/// @brief Shutdown platform-specific systems
void Shutdown();

/// @brief Get the number of logical CPU cores
/// @return Number of logical cores available
u32 GetLogicalCoreCount();

/// @brief Get the number of physical CPU cores
/// @return Number of physical cores available
u32 GetPhysicalCoreCount();

/// @brief Get the page size for virtual memory
/// @return Page size in bytes
usize GetPageSize();

/// @brief Get the cache line size
/// @return Cache line size in bytes (typically 64)
usize GetCacheLineSize();

/// @brief Platform-specific debug break
void DebugBreak();

/// @brief Check if a debugger is attached
/// @return true if debugger is present
bool IsDebuggerPresent();

/// @brief Output a debug string to the debugger
/// @param message The message to output
void OutputDebugString(const char* message);

} // namespace Engine::Platform

