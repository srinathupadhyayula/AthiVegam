#pragma once
#include <cstdint>
#include <vector>
#include <expected>
#include <limits>

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
    EntityLimitReached
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

private:
    WorldOptions _options{};
    std::vector<uint32_t> _versions;   // per-index version counter
    std::vector<uint32_t> _freeList;   // stack of free indices
    std::vector<uint8_t>  _alive;      // 0/1 liveness flag per index
    uint32_t _aliveCount{ 0u };
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
    return Entity{ idx, 1u };
}

inline std::expected<void, Error> World::DestroyEntity(Entity e) noexcept
{
    // Validate and ensure currently alive
    if (e.index >= _versions.size())
        return std::unexpected(Error::InvalidEntity);

    if (_alive[e.index] == 0u || _versions[e.index] != e.version)
        return std::unexpected(Error::AlreadyDestroyed);

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

} // namespace Engine::ECS

