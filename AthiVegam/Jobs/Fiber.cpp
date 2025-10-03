// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - Fiber Implementation
// License: MIT

#include "Jobs/Fiber.hpp"
#include "Core/Logger.hpp"

#ifdef _WIN32
// WIN32_LEAN_AND_MEAN and NOMINMAX are already defined by Core CMakeLists.txt
#include <Windows.h>
#endif

namespace Engine::Jobs
{

#ifdef _WIN32

// Fiber context for storing function
struct FiberContext
{
    FiberFunction fn;
};

// Fiber entry point
static void WINAPI FiberProc(LPVOID param)
{
    auto* context = static_cast<FiberContext*>(param);
    if (context && context->fn)
    {
        context->fn();
    }
}

FiberHandle Fiber::Create(usize stackSize, FiberFunction fn)
{
    auto* context = new FiberContext{std::move(fn)};
    
    LPVOID fiber = CreateFiber(
        stackSize,
        FiberProc,
        context
    );

    if (!fiber)
    {
        LOG_ERROR("Failed to create fiber: {}", GetLastError());
        delete context;
        return nullptr;
    }

    return fiber;
}

void Fiber::Delete(FiberHandle fiber)
{
    if (fiber)
    {
        // Note: We leak the FiberContext here. In a production implementation,
        // we would need to track and clean up the context properly.
        DeleteFiber(fiber);
    }
}

void Fiber::SwitchTo(FiberHandle fiber)
{
    if (fiber)
    {
        SwitchToFiber(fiber);
    }
}

FiberHandle Fiber::ConvertThreadToFiber()
{
    LPVOID fiber = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
    if (!fiber)
    {
        LOG_ERROR("Failed to convert thread to fiber: {}", GetLastError());
    }
    return fiber;
}

void Fiber::ConvertFiberToThread()
{
    // Note: Windows ConvertFiberToThread() returns BOOL, not void
    BOOL result = ::ConvertFiberToThread();
    if (!result)
    {
        LOG_ERROR("Failed to convert fiber to thread: {}", GetLastError());
    }
}

FiberHandle Fiber::GetCurrent()
{
    return GetCurrentFiber();
}

#else

// Stub implementation for non-Windows platforms
FiberHandle Fiber::Create(usize stackSize, FiberFunction fn)
{
    LOG_ERROR("Fibers not implemented for this platform");
    return nullptr;
}

void Fiber::Delete(FiberHandle fiber)
{
    // No-op
}

void Fiber::SwitchTo(FiberHandle fiber)
{
    // No-op
}

FiberHandle Fiber::ConvertThreadToFiber()
{
    LOG_ERROR("Fibers not implemented for this platform");
    return nullptr;
}

void Fiber::ConvertFiberToThread()
{
    // No-op
}

FiberHandle Fiber::GetCurrent()
{
    return nullptr;
}

#endif

} // namespace Engine::Jobs

