#pragma once

#include "Core/Types.hpp"
#include "Core/Memory/Allocators.hpp"

/// @file PoolAllocator.hpp
/// @brief Fixed-size object pool allocator

namespace Engine::Memory {

/// @brief Fixed-size object pool allocator
/// @details Allocates fixed-size blocks from a pre-allocated pool.
///          Fast allocation/deallocation, no fragmentation.
/// @example
/// @code
/// PoolAllocator pool(sizeof(MyObject), alignof(MyObject), 1000);
/// void* ptr = pool.Allocate();
/// // ... use object ...
/// pool.Deallocate(ptr);
/// @endcode
class PoolAllocator : public IAllocator {
public:
    /// @brief Construct pool allocator
    /// @param blockSize Size of each block
    /// @param blockAlignment Alignment of each block
    /// @param blockCount Number of blocks in pool
    PoolAllocator(usize blockSize, usize blockAlignment, usize blockCount);
    
    /// @brief Destructor
    ~PoolAllocator() override;
    
    // Non-copyable, movable
    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;
    PoolAllocator(PoolAllocator&& other) noexcept;
    PoolAllocator& operator=(PoolAllocator&& other) noexcept;
    
    /// @brief Allocate a block
    /// @param size Size to allocate (must match block size)
    /// @param alignment Alignment (must match block alignment)
    /// @return Pointer to allocated block, or nullptr if pool is full
    void* Allocate(usize size, usize alignment = alignof(std::max_align_t)) override;
    
    /// @brief Deallocate a block
    /// @param ptr Pointer to block to deallocate
    void Deallocate(void* ptr) override;
    
    /// @brief Get total allocated bytes
    /// @return Total bytes allocated
    usize Allocated() const override { return _allocatedBlocks * _blockSize; }
    
    /// @brief Get allocator name
    /// @return Allocator name
    const char* Name() const override { return "PoolAllocator"; }
    
    /// @brief Get block size
    /// @return Size of each block
    usize BlockSize() const { return _blockSize; }
    
    /// @brief Get block count
    /// @return Total number of blocks
    usize BlockCount() const { return _blockCount; }
    
    /// @brief Get allocated block count
    /// @return Number of allocated blocks
    usize AllocatedBlocks() const { return _allocatedBlocks; }
    
    /// @brief Get free block count
    /// @return Number of free blocks
    usize FreeBlocks() const { return _blockCount - _allocatedBlocks; }
    
    /// @brief Check if pool is full
    /// @return true if no free blocks
    bool IsFull() const { return _allocatedBlocks >= _blockCount; }
    
    /// @brief Check if pool is empty
    /// @return true if all blocks are free
    bool IsEmpty() const { return _allocatedBlocks == 0; }

private:
    /// @brief Initialize free list by linking all blocks
    void InitializeFreeList();

    byte* _buffer;
    void* _freeList;
    usize _blockSize;
    usize _blockAlignment;
    usize _blockCount;
    usize _allocatedBlocks;
};

} // namespace Engine::Memory

