#include "Core/Memory/Allocators.hpp"

#include <mimalloc.h>
#include <atomic>
#include <spdlog/spdlog.h>

namespace Engine::Memory {

namespace {
    // Global allocation statistics
    struct GlobalStats
    {
        std::atomic<usize> totalAllocated{0};
        std::atomic<usize> totalFreed{0};
        std::atomic<usize> currentUsage{0};
        std::atomic<usize> peakUsage{0};
        std::atomic<usize> allocationCount{0};
        std::atomic<usize> deallocationCount{0};
    };

    GlobalStats _globalStats;
}

AllocationStats GetGlobalStats()
{
    AllocationStats stats;
    stats.totalAllocated = _globalStats.totalAllocated.load(std::memory_order_relaxed);
    stats.totalFreed = _globalStats.totalFreed.load(std::memory_order_relaxed);
    stats.currentUsage = _globalStats.currentUsage.load(std::memory_order_relaxed);
    stats.peakUsage = _globalStats.peakUsage.load(std::memory_order_relaxed);
    stats.allocationCount = _globalStats.allocationCount.load(std::memory_order_relaxed);
    stats.deallocationCount = _globalStats.deallocationCount.load(std::memory_order_relaxed);
    return stats;
}

void ResetGlobalStats()
{
    _globalStats.totalAllocated.store(0, std::memory_order_relaxed);
    _globalStats.totalFreed.store(0, std::memory_order_relaxed);
    _globalStats.currentUsage.store(0, std::memory_order_relaxed);
    _globalStats.peakUsage.store(0, std::memory_order_relaxed);
    _globalStats.allocationCount.store(0, std::memory_order_relaxed);
    _globalStats.deallocationCount.store(0, std::memory_order_relaxed);
}

void* AlignedAlloc(usize size, usize alignment)
{
    if (size == 0)
    {
        return nullptr;
    }

    if (!IsPowerOf2(alignment))
    {
        spdlog::error("Alignment must be a power of 2: {}", alignment);
        return nullptr;
    }

    void* ptr = mi_malloc_aligned(size, alignment);

    if (ptr != nullptr)
    {
        // Update statistics
        _globalStats.totalAllocated.fetch_add(size, std::memory_order_relaxed);
        _globalStats.currentUsage.fetch_add(size, std::memory_order_relaxed);
        _globalStats.allocationCount.fetch_add(1, std::memory_order_relaxed);

        // Update peak usage
        usize currentUsage = _globalStats.currentUsage.load(std::memory_order_relaxed);
        usize peakUsage = _globalStats.peakUsage.load(std::memory_order_relaxed);
        while (currentUsage > peakUsage)
        {
            if (_globalStats.peakUsage.compare_exchange_weak(peakUsage, currentUsage, std::memory_order_relaxed))
            {
                break;
            }
        }
    }

    return ptr;
}

void AlignedFree(void* ptr)
{
    if (ptr == nullptr)
    {
        return;
    }

    // Get allocation size before freeing
    usize size = mi_usable_size(ptr);

    mi_free(ptr);

    // Update statistics
    _globalStats.totalFreed.fetch_add(size, std::memory_order_relaxed);
    _globalStats.currentUsage.fetch_sub(size, std::memory_order_relaxed);
    _globalStats.deallocationCount.fetch_add(1, std::memory_order_relaxed);
}

// Note: IsAligned, AlignUp, AlignDown, and IsPowerOf2 are defined as constexpr inline
// functions in Allocators.hpp and do not need implementations here.

} // namespace Engine::Memory

