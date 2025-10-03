#include "Core/Platform/Platform.hpp"

#include <windows.h>
#include <timeapi.h>
#include <spdlog/spdlog.h>

#pragma comment(lib, "winmm.lib")

namespace Engine::Platform {

namespace {
    bool _initialized = false;
}

bool Initialize()
{
    if (_initialized)
    {
        spdlog::warn("Platform::Initialize() called multiple times");
        return true;
    }

    // Set up Windows-specific initialization
    // Enable high-resolution timer
    timeBeginPeriod(1);

    _initialized = true;
    spdlog::info("Platform initialized successfully");
    return true;
}

void Shutdown()
{
    if (!_initialized)
    {
        spdlog::warn("Platform::Shutdown() called without initialization");
        return;
    }

    // Clean up Windows-specific resources
    timeEndPeriod(1);

    _initialized = false;
    spdlog::info("Platform shut down successfully");
}

u32 GetLogicalCoreCount()
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return static_cast<u32>(sysInfo.dwNumberOfProcessors);
}

u32 GetPhysicalCoreCount()
{
    // Get physical core count using GetLogicalProcessorInformationEx
    DWORD bufferSize = 0;
    GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &bufferSize);

    if (bufferSize == 0)
    {
        spdlog::error("Failed to get processor information buffer size");
        return GetLogicalCoreCount(); // Fallback to logical count
    }

    auto buffer = std::make_unique<u8[]>(bufferSize);
    auto info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.get());

    if (!GetLogicalProcessorInformationEx(RelationProcessorCore, info, &bufferSize))
    {
        spdlog::error("Failed to get processor information");
        return GetLogicalCoreCount(); // Fallback to logical count
    }

    u32 coreCount = 0;
    DWORD offset = 0;

    while (offset < bufferSize)
    {
        auto current = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
            reinterpret_cast<u8*>(info) + offset);

        if (current->Relationship == RelationProcessorCore)
        {
            coreCount++;
        }

        offset += current->Size;
    }

    return coreCount;
}

usize GetPageSize()
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return static_cast<usize>(sysInfo.dwPageSize);
}

usize GetCacheLineSize()
{
    // Return standard cache line size
    // Could be queried via GetLogicalProcessorInformation but 64 is standard
    return 64;
}

void DebugBreak()
{
    __debugbreak();
}

bool IsDebuggerPresent()
{
    return ::IsDebuggerPresent() != 0;
}

void OutputDebugString(const char* message)
{
    if (message == nullptr)
    {
        return;
    }

    ::OutputDebugStringA(message);
}

} // namespace Engine::Platform

