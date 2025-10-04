#include "Core/Memory/PoolAllocator.hpp"
#include "Core/Memory/Allocators.hpp"

#include <spdlog/spdlog.h>
#include <cassert>

namespace Engine::Memory {

PoolAllocator::PoolAllocator(usize blockSize, usize blockAlignment, usize blockCount)
    : _buffer(nullptr)
    , _freeList(nullptr)
    , _blockSize(blockSize)
    , _blockAlignment(blockAlignment)
    , _blockCount(blockCount)
    , _allocatedBlocks(0)
{
    if (blockSize == 0 || blockCount == 0)
    {
        spdlog::error("PoolAllocator blockSize and blockCount must be non-zero");
        return;
    }

    if (!IsPowerOf2(blockAlignment))
    {
        spdlog::error("PoolAllocator alignment must be a power of 2: {}", blockAlignment);
        return;
    }

    // Ensure block size is at least large enough to hold a pointer (for free list)
    if (blockSize < sizeof(void*))
    {
        _blockSize = sizeof(void*);
    }

    // Align block size to alignment
    _blockSize = AlignUp(_blockSize, _blockAlignment);

    // Allocate buffer
    usize totalSize = _blockSize * blockCount;
    _buffer = static_cast<byte*>(AlignedAlloc(totalSize, _blockAlignment));

    if (_buffer == nullptr)
    {
        spdlog::error("Failed to allocate PoolAllocator buffer of size {}", totalSize);
        _blockCount = 0;
        return;
    }

    // Initialize free list
    InitializeFreeList();
}

PoolAllocator::~PoolAllocator()
{
    if (_buffer != nullptr)
    {
        if (_allocatedBlocks > 0)
        {
            spdlog::warn("PoolAllocator destroyed with {} blocks still allocated", _allocatedBlocks);
        }

        AlignedFree(_buffer);
        _buffer = nullptr;
    }
}

void* PoolAllocator::Allocate(usize size, usize alignment)
{
    // Pool allocator only supports fixed-size allocations
    if (size > _blockSize || alignment > _blockAlignment)
    {
        spdlog::error("PoolAllocator allocation size ({}) or alignment ({}) exceeds block size ({}) or alignment ({})",
                     size, alignment, _blockSize, _blockAlignment);
        return nullptr;
    }

    // Check if pool is full
    if (_freeList == nullptr)
    {
        spdlog::error("PoolAllocator is full (all {} blocks allocated)", _blockCount);
        return nullptr;
    }

    // Pop from free list
    void* ptr = _freeList;
    _freeList = *static_cast<void**>(_freeList);
    _allocatedBlocks++;

    return ptr;
}

void PoolAllocator::Deallocate(void* ptr)
{
    if (ptr == nullptr)
    {
        return;
    }

    // Validate that pointer is within pool bounds
    if (ptr < _buffer || ptr >= _buffer + (_blockSize * _blockCount))
    {
        spdlog::error("PoolAllocator::Deallocate called with invalid pointer");
        return;
    }

    // Validate that pointer is aligned to block boundary
    const usize offset = static_cast<byte*>(ptr) - _buffer;
    if (offset % _blockSize != 0)
    {
        spdlog::error("PoolAllocator::Deallocate called with misaligned pointer");
        return;
    }

    // Detect double free by scanning free list (O(n), acceptable for debug safety)
    for (void* node = _freeList; node != nullptr; node = *static_cast<void**>(node))
    {
        if (node == ptr)
        {
            spdlog::error("PoolAllocator::Deallocate detected double free");
            return;
        }
    }

    // Push to free list
    *static_cast<void**>(ptr) = _freeList;
    _freeList = ptr;
    _allocatedBlocks--;
}

// Move constructor
PoolAllocator::PoolAllocator(PoolAllocator&& other) noexcept
    : _buffer(other._buffer)
    , _freeList(other._freeList)
    , _blockSize(other._blockSize)
    , _blockAlignment(other._blockAlignment)
    , _blockCount(other._blockCount)
    , _allocatedBlocks(other._allocatedBlocks)
{
    other._buffer = nullptr;
    other._freeList = nullptr;
    other._blockSize = 0;
    other._blockAlignment = 1;
    other._blockCount = 0;
    other._allocatedBlocks = 0;
}

// Move assignment
PoolAllocator& PoolAllocator::operator=(PoolAllocator&& other) noexcept
{
    if (this != &other)
    {
        // Release current buffer if owned
        if (_buffer != nullptr)
        {
            if (_allocatedBlocks > 0)
            {
                spdlog::warn("PoolAllocator move-assign freeing buffer with {} allocated blocks", _allocatedBlocks);
            }
            AlignedFree(_buffer);
        }

        _buffer = other._buffer;
        _freeList = other._freeList;
        _blockSize = other._blockSize;
        _blockAlignment = other._blockAlignment;
        _blockCount = other._blockCount;
        _allocatedBlocks = other._allocatedBlocks;

        other._buffer = nullptr;
        other._freeList = nullptr;
        other._blockSize = 0;
        other._blockAlignment = 1;
        other._blockCount = 0;
        other._allocatedBlocks = 0;
    }
    return *this;
}

void PoolAllocator::InitializeFreeList()
{
    _freeList = nullptr;

    // Link all blocks into free list (in reverse order for cache efficiency)
    for (usize i = _blockCount; i > 0; --i)
    {
        void* block = _buffer + ((i - 1) * _blockSize);
        *static_cast<void**>(block) = _freeList;
        _freeList = block;
    }
}

} // namespace Engine::Memory

