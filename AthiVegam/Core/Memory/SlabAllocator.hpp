#pragma once

#include "Core/Types.hpp"
#include "Core/Memory/Allocators.hpp"
#include "Core/Memory/Handle.hpp"
#include <vector>

/// @file SlabAllocator.hpp
/// @brief Slab allocator with versioned handles

namespace Engine::Memory {

/// @brief Slab allocator with versioned handles
/// @details Allocates fixed-size objects with stable versioned handles.
///          Handles remain valid across reallocations via version checking.
/// @tparam T Type of objects to allocate
/// @example
/// @code
/// SlabAllocator<MyObject> slab(1000);
/// Handle<MyObject> handle = slab.Allocate();
/// MyObject* obj = slab.Get(handle);
/// // ... use object ...
/// slab.Deallocate(handle);
/// @endcode
template<typename T>
class SlabAllocator {
public:
    /// @brief Construct slab allocator
    /// @param initialCapacity Initial number of slots
    explicit SlabAllocator(usize initialCapacity = 256);
    
    /// @brief Destructor
    ~SlabAllocator();
    
    // Non-copyable, movable
    SlabAllocator(const SlabAllocator&) = delete;
    SlabAllocator& operator=(const SlabAllocator&) = delete;
    SlabAllocator(SlabAllocator&& other) noexcept;
    SlabAllocator& operator=(SlabAllocator&& other) noexcept;
    
    /// @brief Allocate a new object
    /// @return Handle to allocated object
    Handle<T> Allocate();
    
    /// @brief Deallocate an object
    /// @param handle Handle to object to deallocate
    void Deallocate(Handle<T> handle);
    
    /// @brief Get object from handle
    /// @param handle Handle to object
    /// @return Pointer to object, or nullptr if handle is invalid
    T* Get(Handle<T> handle);
    
    /// @brief Get const object from handle
    /// @param handle Handle to object
    /// @return Const pointer to object, or nullptr if handle is invalid
    const T* Get(Handle<T> handle) const;
    
    /// @brief Check if handle is valid
    /// @param handle Handle to check
    /// @return true if handle points to a valid object
    bool IsValid(Handle<T> handle) const;
    
    /// @brief Get number of allocated objects
    /// @return Number of allocated objects
    usize AllocatedCount() const { return _allocatedCount; }
    
    /// @brief Get capacity
    /// @return Total capacity
    usize Capacity() const { return _slots.size(); }
    
    /// @brief Clear all allocations
    void Clear();
    
private:
    struct Slot {
        T object;
        u32 version;
        bool occupied;
    };
    
    std::vector<Slot> _slots;
    std::vector<u32> _freeList;
    usize _allocatedCount;
};

// Template implementation
template<typename T>
SlabAllocator<T>::SlabAllocator(usize initialCapacity)
    : _allocatedCount(0)
{
    _slots.reserve(initialCapacity);
    _freeList.reserve(initialCapacity);
}

template<typename T>
SlabAllocator<T>::~SlabAllocator()
{
    Clear();
}

template<typename T>
SlabAllocator<T>::SlabAllocator(SlabAllocator&& other) noexcept
    : _slots(std::move(other._slots))
    , _freeList(std::move(other._freeList))
    , _allocatedCount(other._allocatedCount)
{
    other._allocatedCount = 0;
}

template<typename T>
SlabAllocator<T>& SlabAllocator<T>::operator=(SlabAllocator&& other) noexcept
{
    if (this != &other) {
        Clear();
        _slots = std::move(other._slots);
        _freeList = std::move(other._freeList);
        _allocatedCount = other._allocatedCount;
        other._allocatedCount = 0;
    }
    return *this;
}

template<typename T>
Handle<T> SlabAllocator<T>::Allocate()
{
    u32 index;

    if (!_freeList.empty()) {
        // Reuse free slot
        index = _freeList.back();
        _freeList.pop_back();
        // Reconstruct object to a known default state
        new (&_slots[index].object) T();
    } else {
        // Allocate new slot (default constructs T as part of Slot)
        index = static_cast<u32>(_slots.size());
        _slots.emplace_back();
        _slots[index].version = 1;
    }

    _slots[index].occupied = true;
    ++_allocatedCount;

    return Handle<T>(index, _slots[index].version);
}

template<typename T>
void SlabAllocator<T>::Deallocate(Handle<T> handle)
{
    if (!IsValid(handle)) {
        return;
    }

    u32 index = handle.Index();
    // Invoke destructor to release any resources held by T
    _slots[index].object.~T();
    _slots[index].occupied = false;
    _slots[index].version++;
    _freeList.push_back(index);
    --_allocatedCount;
}

template<typename T>
T* SlabAllocator<T>::Get(Handle<T> handle)
{
    if (!IsValid(handle)) {
        return nullptr;
    }
    
    return &_slots[handle.Index()].object;
}

template<typename T>
const T* SlabAllocator<T>::Get(Handle<T> handle) const
{
    if (!IsValid(handle)) {
        return nullptr;
    }
    
    return &_slots[handle.Index()].object;
}

template<typename T>
bool SlabAllocator<T>::IsValid(Handle<T> handle) const
{
    if (!handle.IsValid()) {
        return false;
    }
    
    u32 index = handle.Index();
    if (index >= _slots.size()) {
        return false;
    }
    
    return _slots[index].occupied && _slots[index].version == handle.Version();
}

template<typename T>
void SlabAllocator<T>::Clear()
{
    _slots.clear();
    _freeList.clear();
    _allocatedCount = 0;
}

} // namespace Engine::Memory

