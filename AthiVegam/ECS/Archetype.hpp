#pragma once
#include "ComponentTraits.hpp"
#include "ComponentRegistry.hpp"
#include <unordered_map>
#include <memory>
#include <cstring>

namespace Engine::ECS {

// Forward declarations
class Chunk;
class Archetype;

// Custom deleter for aligned memory
struct AlignedDeleter {
    void operator()(uint8_t* ptr) const noexcept {
        ::operator delete[](ptr, std::align_val_t{ 64 });
    }
};

// Forward declaration for friend
class World;

// Chunk: SoA storage for entities with the same component signature
// Size: 32-64 KB for cache efficiency; 64-byte aligned for SIMD
class Chunk {
public:
    static constexpr size_t CHUNK_SIZE = 64 * 1024; // 64 KB
    static constexpr size_t ALIGNMENT = 64;         // 64-byte alignment for SIMD

    explicit Chunk(const ComponentSignature& signature);
    ~Chunk() = default;

    // Non-copyable, movable
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;
    Chunk(Chunk&&) noexcept = default;
    Chunk& operator=(Chunk&&) noexcept = default;

    // Allow World to access internals for component migration
    friend class World;

    // Get component column pointer for type T
    template<Component T>
    [[nodiscard]] T* GetColumn() noexcept
    {
        const auto typeID = GetComponentTypeID<T>();
        auto it = _columnOffsets.find(typeID);
        if (it == _columnOffsets.end())
            return nullptr;
        return reinterpret_cast<T*>(_data.get() + it->second);
    }

    // Get component for entity at index
    template<Component T>
    [[nodiscard]] T* GetComponent(uint32_t index) noexcept
    {
        T* column = GetColumn<T>();
        if (!column || index >= _count)
            return nullptr;
        return &column[index];
    }

    // Add entity to chunk (returns index, or -1 if full)
    [[nodiscard]] int32_t AddEntity(uint32_t entityIndex) noexcept;

    // Remove entity from chunk at index (swaps with last)
    // Returns the entity index that was swapped (or 0 if no swap occurred)
    uint32_t RemoveEntity(uint32_t index) noexcept;

    // Current entity count in chunk
    [[nodiscard]] uint32_t Count() const noexcept { return _count; }

    // Maximum entities this chunk can hold
    [[nodiscard]] uint32_t Capacity() const noexcept { return _capacity; }

    // Check if chunk is full
    [[nodiscard]] bool IsFull() const noexcept { return _count >= _capacity; }

    // Get entity index at chunk position
    [[nodiscard]] uint32_t GetEntityIndex(uint32_t chunkIndex) const noexcept
    {
        return chunkIndex < _count ? _entityIndices[chunkIndex] : 0;
    }

private:
    void CalculateLayout(const ComponentSignature& signature);

    std::unique_ptr<uint8_t[], AlignedDeleter> _data;          // Aligned chunk memory
    std::unordered_map<ComponentTypeID, size_t> _columnOffsets; // Type ID → byte offset
    std::vector<size_t> _componentSizes;                       // Size of each component
    std::vector<uint32_t> _entityIndices;                      // Entity index per slot
    uint32_t _count{ 0 };                                      // Current entity count
    uint32_t _capacity{ 0 };                                   // Max entities in chunk
};

// Archetype: represents a unique component signature and manages chunks
class Archetype {
public:
    explicit Archetype(ComponentSignature signature)
        : _signature(std::move(signature))
    {
    }

    // Get or create a chunk with available space
    [[nodiscard]] Chunk* GetAvailableChunk();

    // Get all chunks
    [[nodiscard]] const std::vector<std::unique_ptr<Chunk>>& GetChunks() const noexcept
    {
        return _chunks;
    }

    // Get component signature
    [[nodiscard]] const ComponentSignature& GetSignature() const noexcept
    {
        return _signature;
    }

    // Find entity in archetype (returns chunk index and entity index within chunk)
    struct EntityLocation {
        Chunk* chunk{ nullptr };
        uint32_t indexInChunk{ 0 };
    };

    [[nodiscard]] EntityLocation FindEntity(uint32_t entityIndex) const noexcept;

private:
    ComponentSignature _signature;
    std::vector<std::unique_ptr<Chunk>> _chunks;
};

// ===== Inline implementations =====

inline Chunk::Chunk(const ComponentSignature& signature)
{
    CalculateLayout(signature);

    // Allocate aligned memory using operator new[] with alignment
    uint8_t* rawPtr = static_cast<uint8_t*>(::operator new[](CHUNK_SIZE, std::align_val_t{ ALIGNMENT }));
    _data = std::unique_ptr<uint8_t[], AlignedDeleter>(rawPtr);
    std::memset(_data.get(), 0, CHUNK_SIZE);
}

inline void Chunk::CalculateLayout(const ComponentSignature& signature)
{
    const auto& typeIDs = signature.GetTypeIDs();
    if (typeIDs.empty())
    {
        _capacity = 0;
        return;
    }

    auto& registry = ComponentRegistry::Instance();

    // Calculate component sizes and total per-entity size
    size_t perEntitySize = sizeof(uint32_t); // Entity index
    for (const auto typeID : typeIDs)
    {
        const auto* meta = registry.GetMetadata(typeID);
        if (!meta)
        {
            // Component not registered - this is a programming error
            _capacity = 0;
            return;
        }
        _componentSizes.push_back(meta->size);
        perEntitySize += meta->size;
    }

    // Calculate capacity based on chunk size
    _capacity = static_cast<uint32_t>((CHUNK_SIZE - 256) / perEntitySize); // Reserve 256 bytes for metadata

    // Layout: [entity indices][component columns...]
    size_t entityIndicesSize = _capacity * sizeof(uint32_t);
    size_t offset = entityIndicesSize;

    // Align offset to 64 bytes
    offset = (offset + ALIGNMENT - 1) & ~(ALIGNMENT - 1);

    // Calculate column offsets
    for (size_t i = 0; i < typeIDs.size(); ++i)
    {
        _columnOffsets[typeIDs[i]] = offset;
        offset += _componentSizes[i] * _capacity;
        // Align each column to 64 bytes
        offset = (offset + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    }

    _entityIndices.resize(_capacity, 0);
}

inline int32_t Chunk::AddEntity(uint32_t entityIndex) noexcept
{
    if (IsFull())
        return -1;

    const uint32_t index = _count++;
    _entityIndices[index] = entityIndex;
    return static_cast<int32_t>(index);
}

inline uint32_t Chunk::RemoveEntity(uint32_t index) noexcept
{
    if (index >= _count)
        return 0;

    // Swap with last entity (preserves cache locality)
    const uint32_t lastIndex = _count - 1;
    uint32_t swappedEntityIndex = 0;

    if (index != lastIndex)
    {
        swappedEntityIndex = _entityIndices[lastIndex];
        _entityIndices[index] = swappedEntityIndex;

        // Swap component data for all columns
        size_t componentIndex = 0;
        for (const auto& [typeID, offset] : _columnOffsets)
        {
            const size_t componentSize = _componentSizes[componentIndex++];
            uint8_t* column = _data.get() + offset;
            std::memcpy(column + index * componentSize,
                       column + lastIndex * componentSize,
                       componentSize);
        }
    }

    _count--;
    return swappedEntityIndex;
}

inline Chunk* Archetype::GetAvailableChunk()
{
    // Find existing chunk with space
    for (auto& chunk : _chunks)
    {
        if (!chunk->IsFull())
            return chunk.get();
    }

    // Create new chunk
    _chunks.push_back(std::make_unique<Chunk>(_signature));
    return _chunks.back().get();
}

inline Archetype::EntityLocation Archetype::FindEntity(uint32_t entityIndex) const noexcept
{
    for (const auto& chunk : _chunks)
    {
        for (uint32_t i = 0; i < chunk->Count(); ++i)
        {
            if (chunk->GetEntityIndex(i) == entityIndex)
            {
                return EntityLocation{ chunk.get(), i };
            }
        }
    }
    return EntityLocation{};
}

} // namespace Engine::ECS

