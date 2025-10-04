// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - Fiber Abstraction
// License: MIT

#pragma once

#include "Core/Types.hpp"
#include <functional>

namespace Engine::Jobs
{

/// @brief Fiber handle (platform-specific)
#ifdef _WIN32
using FiberHandle = void*;
#else
using FiberHandle = void*; // TODO: Implement for other platforms
#endif

/// @brief Fiber function signature
using FiberFunction = std::function<void()>;

/// @brief Fiber abstraction for cooperative multitasking
/// @details Wraps platform-specific fiber APIs (Windows Fibers, etc.)
///          Allows blocking operations without stalling OS threads.
/// @note Currently only implemented for Windows
class Fiber
{
public:
    /// @brief Create a fiber
    /// @param stackSize Stack size in bytes (0 = default)
    /// @param fn Function to execute in the fiber
    /// @return Fiber handle or nullptr on failure
    static FiberHandle Create(usize stackSize, FiberFunction fn);

    /// @brief Delete a fiber
    /// @param fiber Fiber handle to delete
    static void Delete(FiberHandle fiber);

    /// @brief Switch to a fiber
    /// @param fiber Fiber handle to switch to
    static void SwitchTo(FiberHandle fiber);

    /// @brief Convert current thread to a fiber
    /// @return Fiber handle for the current thread
    static FiberHandle ConvertThreadToFiber();

    /// @brief Convert current fiber back to a thread
    static void ConvertFiberToThread();

    /// @brief Get the currently executing fiber
    /// @return Current fiber handle
    static FiberHandle GetCurrent();
};

} // namespace Engine::Jobs

