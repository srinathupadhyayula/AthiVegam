#pragma once
#include "ComponentTraits.hpp"
#include "ComponentRegistry.hpp"
#include "Archetype.hpp"
#include "Query.hpp"
#include <cstdint>
#include <vector>
#include <expected>
#include <limits>
#include <unordered_map>

namespace Engine::ECS {

// Entity identifier: 64-bit composed of 32-bit index and 32-bit version
struct Entity {
    uint32_t index{ std::numeric_limits<uint32_t>::max() };
    uint32_t version{ 0u };
};

// Error codes for ECS world operations
enum class Error {
    InvalidEntity,
    AlreadyDestroyed,
    EntityLimitReached,
    ComponentNotFound,
    ComponentAlreadyExists
};

// WorldOptions allow configuring max entities (0 = unbounded growth)
struct WorldOptions {
    uint32_t maxEntities{ 0u };
};

// Simple entity lifecycle manager using a free-list and versioned IDs.
// - On Destroy: mark as not alive and increment the version counter; push index to free-list.
// - On Create: reuse index from free-list (if any) with its incremented version; otherwise append a new slot.
class World {
public:
    World() = default;
    explicit World(WorldOptions options) : _options(options) {}

    // Create a new entity. Reuses an index from the free-list when available.
    [[nodiscard]] Entity CreateEntity() noexcept;

    // Destroy an entity; returns AlreadyDestroyed/InvalidEntity on error.
    std::expected<void, Error> DestroyEntity(Entity e) noexcept;

    // Returns true if the given entity is currently alive in this world.
    [[nodiscard]] bool IsAlive(Entity e) const noexcept;

    // Validate an entity; returns InvalidEntity when not alive or out of range.
    std::expected<void, Error> Validate(Entity e) const noexcept;

    // Counts
    [[nodiscard]] uint32_t AliveCount() const noexcept { return _aliveCount; }
    [[nodiscard]] uint32_t Capacity() const noexcept { return static_cast<uint32_t>(_versions.size()); }

    // Component operations
    template<Component T>
    std::expected<void, Error> Add(Entity e, const T& value);

    template<Component T>
    std::expected<void, Error> Remove(Entity e);

    template<Component T>
    std::expected<T*, Error> Get(Entity e);

    template<Component T>
    [[nodiscard]] bool Has(Entity e) const noexcept;

    // Query system - iterate over entities with specific components
    template<Component... Ts>
    [[nodiscard]] Query<Ts...> QueryComponents() const noexcept;

    template<Component... IncludeTs, Component... ExcludeTs>
    [[nodiscard]] Query<IncludeTs...> QueryComponents(Exclude<ExcludeTs...>) const noexcept;

private:
    // Entity location tracking
    struct EntityRecord {
        Archetype* archetype{ nullptr };
        Chunk* chunk{ nullptr };
        uint32_t indexInChunk{ 0 };
    };

    // Get or create archetype for signature
    Archetype* GetOrCreateArchetype(const ComponentSignature& signature);

    // Move entity to new archetype (used during Add/Remove component)
    void MoveEntity(Entity e, Archetype* newArchetype);

    WorldOptions _options{};
    std::vector<uint32_t> _versions;   // per-index version counter
    std::vector<uint32_t> _freeList;   // stack of free indices
    std::vector<uint8_t>  _alive;      // 0/1 liveness flag per index
    uint32_t _aliveCount{ 0u };

    // Archetype management
    std::unordered_map<ComponentSignature, std::unique_ptr<Archetype>> _archetypes;
    std::vector<EntityRecord> _entityRecords; // Parallel to entity indices
};

// ===== Inline implementation =====
inline Entity World::CreateEntity() noexcept
{
    uint32_t idx;

    if (!_freeList.empty())
    {
        // Reuse a previously destroyed slot
        idx = _freeList.back();
        _freeList.pop_back();
        // Version was incremented on DestroyEntity()
        _alive[idx] = 1u;
        _aliveCount++;

        // Ensure entity record exists
        if (idx >= _entityRecords.size())
            _entityRecords.resize(idx + 1);

        return Entity{ idx, _versions[idx] };
    }

    // Capacity check if bounded
    if (_options.maxEntities != 0u && Capacity() >= _options.maxEntities)
    {
        return Entity{}; // invalid sentinel; caller can detect via Validate if needed
    }

    // Append a new slot with version starting at 1
    idx = static_cast<uint32_t>(_versions.size());
    _versions.push_back(1u);
    _alive.push_back(1u);
    _aliveCount++;
    _entityRecords.resize(idx + 1);
    return Entity{ idx, 1u };
}

inline std::expected<void, Error> World::DestroyEntity(Entity e) noexcept
{
    // Validate and ensure currently alive
    if (e.index >= _versions.size())
        return std::unexpected(Error::InvalidEntity);

    if (_alive[e.index] == 0u || _versions[e.index] != e.version)
        return std::unexpected(Error::AlreadyDestroyed);

    // Remove from archetype/chunk if present
    if (e.index < _entityRecords.size())
    {
        auto& record = _entityRecords[e.index];
        if (record.chunk)
        {
            uint32_t swappedEntityIndex = record.chunk->RemoveEntity(record.indexInChunk);

            // If an entity was swapped, update its record
            if (swappedEntityIndex != 0 && swappedEntityIndex < _entityRecords.size())
            {
                _entityRecords[swappedEntityIndex].indexInChunk = record.indexInChunk;
            }

            record.archetype = nullptr;
            record.chunk = nullptr;
            record.indexInChunk = 0;
        }
    }

    // Mark not alive and increment version to invalidate outstanding handles
    _alive[e.index] = 0u;
    _aliveCount--;
    _versions[e.index] += 1u; // wrap-around is acceptable; mismatch still invalidates

    // Push back to free-list for reuse
    _freeList.push_back(e.index);
    return {};
}

inline bool World::IsAlive(Entity e) const noexcept
{
    if (e.index >= _versions.size())
        return false;
    return (_alive[e.index] != 0u) && (_versions[e.index] == e.version);
}

inline std::expected<void, Error> World::Validate(Entity e) const noexcept
{
    if (!IsAlive(e))
        return std::unexpected(Error::InvalidEntity);
    return {};
}

// Component operations
template<Component T>
inline std::expected<void, Error> World::Add(Entity e, const T& value)
{
    // Validate entity
    auto validResult = Validate(e);
    if (!validResult.has_value())
        return validResult;

    // Ensure component is registered
    ComponentRegistry::Instance().Register<T>();

    // Check if entity already has this component
    if (Has<T>(e))
        return std::unexpected(Error::ComponentAlreadyExists);

    // Get current signature and create new signature with added component
    ComponentSignature newSignature;
    if (e.index < _entityRecords.size() && _entityRecords[e.index].archetype)
    {
        newSignature = _entityRecords[e.index].archetype->GetSignature();
    }
    newSignature.Add<T>();

    // Get or create archetype for new signature
    Archetype* newArchetype = GetOrCreateArchetype(newSignature);

    // Move entity to new archetype
    MoveEntity(e, newArchetype);

    // Set component value
    auto componentResult = Get<T>(e);
    if (componentResult.has_value())
    {
        *componentResult.value() = value;
    }

    return {};
}

template<Component T>
inline std::expected<void, Error> World::Remove(Entity e)
{
    // Validate entity
    auto validResult = Validate(e);
    if (!validResult.has_value())
        return validResult;

    // Check if entity has this component
    if (!Has<T>(e))
        return std::unexpected(Error::ComponentNotFound);

    // Get current signature and create new signature without component
    ComponentSignature newSignature = _entityRecords[e.index].archetype->GetSignature();
    newSignature.Remove<T>();

    // Get or create archetype for new signature
    Archetype* newArchetype = GetOrCreateArchetype(newSignature);

    // Move entity to new archetype
    MoveEntity(e, newArchetype);

    return {};
}

template<Component T>
inline std::expected<T*, Error> World::Get(Entity e)
{
    // Validate entity
    auto validResult = Validate(e);
    if (!validResult.has_value())
        return std::unexpected(validResult.error());

    // Check if entity has archetype/chunk
    if (e.index >= _entityRecords.size())
        return std::unexpected(Error::ComponentNotFound);

    auto& record = _entityRecords[e.index];
    if (!record.chunk)
        return std::unexpected(Error::ComponentNotFound);

    // Get component from chunk
    T* component = record.chunk->GetComponent<T>(record.indexInChunk);
    if (!component)
        return std::unexpected(Error::ComponentNotFound);

    return component;
}

template<Component T>
inline bool World::Has(Entity e) const noexcept
{
    if (!IsAlive(e))
        return false;

    if (e.index >= _entityRecords.size())
        return false;

    const auto& record = _entityRecords[e.index];
    if (!record.archetype)
        return false;

    return record.archetype->GetSignature().Contains<T>();
}

inline Archetype* World::GetOrCreateArchetype(const ComponentSignature& signature)
{
    auto it = _archetypes.find(signature);
    if (it != _archetypes.end())
        return it->second.get();

    auto archetype = std::make_unique<Archetype>(signature);
    auto* ptr = archetype.get();
    _archetypes[signature] = std::move(archetype);
    return ptr;
}

inline void World::MoveEntity(Entity e, Archetype* newArchetype)
{
    auto& record = _entityRecords[e.index];

    Chunk* oldChunk = record.chunk;
    uint32_t oldIndex = record.indexInChunk;
    Archetype* oldArchetype = record.archetype;

    // Get available chunk in new archetype
    Chunk* newChunk = newArchetype->GetAvailableChunk();
    int32_t newIndex = newChunk->AddEntity(e.index);

    if (newIndex < 0)
    {
        // Chunk full - this shouldn't happen as GetAvailableChunk ensures space
        return;
    }

    // Update entity record BEFORE removing from old chunk
    // (RemoveEntity may swap entities and we need the record to be correct)
    record.archetype = newArchetype;
    record.chunk = newChunk;
    record.indexInChunk = static_cast<uint32_t>(newIndex);

    // Copy component data from old chunk to new chunk (if entity had components)
    if (oldChunk && oldArchetype)
    {
        const auto& oldSig = oldArchetype->GetSignature();
        const auto& newSig = newArchetype->GetSignature();

        // Copy components that exist in both signatures
        for (const auto typeID : oldSig.GetTypeIDs())
        {
            if (std::ranges::find(newSig.GetTypeIDs(), typeID) != newSig.GetTypeIDs().end())
            {
                // Component exists in both - copy data
                const auto* meta = ComponentRegistry::Instance().GetMetadata(typeID);
                if (meta && meta->copyConstruct)
                {
                    // Find column info in both chunks
                    const auto& oldColumns = oldChunk->GetColumns();
                    const auto& newColumns = newChunk->GetColumns();
                    auto oldColumnIt = std::ranges::find_if(oldColumns,
                        [typeID](const auto& col) { return col.typeID == typeID; });
                    auto newColumnIt = std::ranges::find_if(newColumns,
                        [typeID](const auto& col) { return col.typeID == typeID; });

                    if (oldColumnIt != oldColumns.end() &&
                        newColumnIt != newColumns.end())
                    {
                        // Calculate pointers to old and new component data
                        uint8_t* oldData = oldChunk->_data.get() + oldColumnIt->offset + (oldIndex * meta->size);
                        uint8_t* newData = newChunk->_data.get() + newColumnIt->offset + (newIndex * meta->size);

                        // Copy component data
                        std::memcpy(newData, oldData, meta->size);
                    }
                }
            }
        }

        // Remove from old chunk (this will swap with last entity)
        uint32_t swappedEntityIndex = oldChunk->RemoveEntity(oldIndex);

        // Update the swapped entity's record if one was swapped
        if (swappedEntityIndex != 0 && swappedEntityIndex < _entityRecords.size())
        {
            _entityRecords[swappedEntityIndex].indexInChunk = oldIndex;
        }
    }
}

// Query system implementations
template<Component... Ts>
inline Query<Ts...> World::QueryComponents() const noexcept
{
    std::vector<Archetype*> matchingArchetypes;

    // Find all archetypes that contain all required components
    for (const auto& [signature, archetype] : _archetypes)
    {
        if (SignatureMatches<Ts...>(signature))
        {
            matchingArchetypes.push_back(archetype.get());
        }
    }

    return Query<Ts...>(std::move(matchingArchetypes));
}

template<Component... IncludeTs, Component... ExcludeTs>
inline Query<IncludeTs...> World::QueryComponents(Exclude<ExcludeTs...> exclude) const noexcept
{
    std::vector<Archetype*> matchingArchetypes;

    // Find all archetypes that contain all required components and none of the excluded ones
    for (const auto& [signature, archetype] : _archetypes)
    {
        if (SignatureMatches<IncludeTs...>(signature, exclude))
        {
            matchingArchetypes.push_back(archetype.get());
        }
    }

    return Query<IncludeTs...>(std::move(matchingArchetypes));
}

} // namespace Engine::ECS

