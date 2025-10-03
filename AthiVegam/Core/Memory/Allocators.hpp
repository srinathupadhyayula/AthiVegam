#pragma once

#include "Core/Types.hpp"

/// @file Allocators.hpp
/// @brief Memory allocator interfaces and utilities

namespace Engine::Memory {

/// @brief Base allocator interface
class IAllocator {
public:
    virtual ~IAllocator() = default;

    /// @brief Allocate memory
    /// @param size Size in bytes
    /// @param alignment Alignment requirement
    /// @return Pointer to allocated memory, or nullptr on failure
    virtual void* Allocate(usize size, usize alignment = alignof(std::max_align_t)) = 0;

    /// @brief Deallocate memory
    /// @param ptr Pointer to memory to deallocate
    virtual void Deallocate(void* ptr) = 0;

    /// @brief Get total allocated bytes
    /// @return Total bytes allocated
    virtual usize Allocated() const = 0;

    /// @brief Get allocator name for debugging
    /// @return Allocator name
    virtual const char* Name() const = 0;
};

/// @brief Allocation statistics
struct AllocationStats {
    usize totalAllocated = 0;      ///< Total bytes allocated
    usize totalFreed = 0;           ///< Total bytes freed
    usize currentUsage = 0;         ///< Current memory usage
    usize peakUsage = 0;            ///< Peak memory usage
    usize allocationCount = 0;      ///< Number of allocations
    usize deallocationCount = 0;    ///< Number of deallocations
};

/// @brief Get global allocation statistics
/// @return Global allocation stats
AllocationStats GetGlobalStats();

/// @brief Reset global allocation statistics
void ResetGlobalStats();

/// @brief Aligned allocation
/// @param size Size in bytes
/// @param alignment Alignment requirement (must be power of 2)
/// @return Pointer to aligned memory, or nullptr on failure
void* AlignedAlloc(usize size, usize alignment);

/// @brief Aligned deallocation
/// @param ptr Pointer to aligned memory
void AlignedFree(void* ptr);

/// @brief Check if pointer is aligned
/// @param ptr Pointer to check
/// @param alignment Alignment requirement
/// @return true if aligned
inline bool IsAligned(const void* ptr, usize alignment) {
    return (reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)) == 0;
}

/// @brief Align value up to alignment
/// @param value Value to align
/// @param alignment Alignment (must be power of 2)
/// @return Aligned value
constexpr usize AlignUp(usize value, usize alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

/// @brief Align value down to alignment
/// @param value Value to align
/// @param alignment Alignment (must be power of 2)
/// @return Aligned value
constexpr usize AlignDown(usize value, usize alignment) {
    return value & ~(alignment - 1);
}

/// @brief Check if value is power of 2
/// @param value Value to check
/// @return true if power of 2
constexpr bool IsPowerOf2(usize value) {
    return value != 0 && (value & (value - 1)) == 0;
}

} // namespace Engine::Memory

