#pragma once

#include "Core/Types.hpp"
#include "Core/Memory/Allocators.hpp"

/// @file FrameArena.hpp
/// @brief Frame-scoped bump allocator

namespace Engine::Memory {

/// @brief Frame-scoped bump allocator
/// @details Allocates memory with bump pointer, resets each frame.
///          Fast allocation, no individual deallocation.
/// @example
/// @code
/// FrameArena arena(1 * MiB);
/// void* ptr = arena.Allocate(64, 16);
/// // ... use memory ...
/// arena.Reset(); // Free all at once
/// @endcode
class FrameArena : public IAllocator {
public:
    /// @brief Construct arena with capacity
    /// @param capacity Maximum bytes to allocate
    explicit FrameArena(usize capacity);
    
    /// @brief Destructor
    ~FrameArena() override;
    
    // Non-copyable, movable
    FrameArena(const FrameArena&) = delete;
    FrameArena& operator=(const FrameArena&) = delete;
    FrameArena(FrameArena&& other) noexcept;
    FrameArena& operator=(FrameArena&& other) noexcept;
    
    /// @brief Allocate aligned memory
    /// @param size Bytes to allocate
    /// @param alignment Alignment requirement (power of 2)
    /// @return Pointer to allocated memory, or nullptr if out of space
    void* Allocate(usize size, usize alignment = alignof(std::max_align_t)) override;
    
    /// @brief Deallocate memory (no-op for arena)
    /// @param ptr Pointer to deallocate (ignored)
    void Deallocate(void* ptr) override;
    
    /// @brief Get total allocated bytes
    /// @return Total bytes allocated
    usize Allocated() const override { return _offset; }
    
    /// @brief Get allocator name
    /// @return Allocator name
    const char* Name() const override { return "FrameArena"; }
    
    /// @brief Reset allocator (frees all allocations)
    void Reset();
    
    /// @brief Get capacity
    /// @return Maximum capacity in bytes
    usize Capacity() const { return _capacity; }
    
    /// @brief Get used bytes
    /// @return Currently used bytes
    usize Used() const { return _offset; }
    
    /// @brief Get remaining bytes
    /// @return Remaining capacity
    usize Remaining() const { return _capacity - _offset; }
    
private:
    byte* _buffer;
    usize _capacity;
    usize _offset;
};

} // namespace Engine::Memory

