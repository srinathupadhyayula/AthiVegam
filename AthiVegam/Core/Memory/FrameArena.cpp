#include "Core/Memory/FrameArena.hpp"
#include "Core/Memory/Allocators.hpp"

#include <spdlog/spdlog.h>
#include <cassert>

namespace Engine::Memory {

FrameArena::FrameArena(usize capacity)
    : _buffer(nullptr)
    , _capacity(capacity)
    , _offset(0)
{
    if (capacity == 0)
    {
        spdlog::error("FrameArena capacity cannot be zero");
        return;
    }

    // Allocate aligned buffer
    _buffer = static_cast<byte*>(AlignedAlloc(capacity, CacheLineSize));

    if (_buffer == nullptr)
    {
        spdlog::error("Failed to allocate FrameArena buffer of size {}", capacity);
        _capacity = 0;
    }
}

FrameArena::~FrameArena()
{
    if (_buffer != nullptr)
    {
        AlignedFree(_buffer);
        _buffer = nullptr;
    }
}

// Move constructor
FrameArena::FrameArena(FrameArena&& other) noexcept
    : _buffer(other._buffer)
    , _capacity(other._capacity)
    , _offset(other._offset)
{
    other._buffer = nullptr;
    other._capacity = 0;
    other._offset = 0;
}

// Move assignment
FrameArena& FrameArena::operator=(FrameArena&& other) noexcept
{
    if (this != &other)
    {
        if (_buffer != nullptr)
        {
            AlignedFree(_buffer);
        }
        _buffer = other._buffer;
        _capacity = other._capacity;
        _offset = other._offset;

        other._buffer = nullptr;
        other._capacity = 0;
        other._offset = 0;
    }
    return *this;
}

void* FrameArena::Allocate(usize size, usize alignment)
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

    // Align the current offset
    usize alignedOffset = AlignUp(_offset, alignment);

    // Check if we have enough space
    if (alignedOffset + size > _capacity)
    {
        spdlog::error("FrameArena out of memory (requested: {}, available: {})", 
                     size, _capacity - alignedOffset);
        return nullptr;
    }

    // Allocate from the arena
    void* ptr = _buffer + alignedOffset;
    _offset = alignedOffset + size;

    return ptr;
}

void FrameArena::Deallocate(void* ptr)
{
    // FrameArena doesn't support individual deallocation
    // Memory is freed in bulk via Reset()
    (void)ptr;
}

void FrameArena::Reset()
{
    _offset = 0;
}

} // namespace Engine::Memory

