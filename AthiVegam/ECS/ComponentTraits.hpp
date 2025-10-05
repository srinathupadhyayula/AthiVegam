#pragma once
#include <concepts>
#include <cstdint>
#include <type_traits>
#include <typeindex>
#include <algorithm>
#include <vector>

namespace Engine::ECS {

// Component concept: must be trivially copyable and standard layout for SoA storage
template<typename T>
concept Component = std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

// Compile-time type ID generation using type_index hash
// This provides a stable, unique identifier for each component type
using ComponentTypeID = size_t;

template<Component T>
constexpr ComponentTypeID GetComponentTypeID() noexcept
{
    return typeid(T).hash_code();
}

// Component signature: sorted list of type IDs representing an archetype
// Sorted for fast comparison and deterministic archetype matching
class ComponentSignature {
public:
    ComponentSignature() = default;

    // Add a component type to the signature
    template<Component T>
    void Add() noexcept
    {
        const auto typeID = GetComponentTypeID<T>();
        if (std::ranges::find(_typeIDs, typeID) == _typeIDs.end())
        {
            _typeIDs.push_back(typeID);
            std::ranges::sort(_typeIDs);
        }
    }

    // Remove a component type from the signature
    template<Component T>
    void Remove() noexcept
    {
        const auto typeID = GetComponentTypeID<T>();
        auto it = std::ranges::find(_typeIDs, typeID);
        if (it != _typeIDs.end())
        {
            _typeIDs.erase(it);
        }
    }

    // Check if signature contains a component type
    template<Component T>
    [[nodiscard]] bool Contains() const noexcept
    {
        const auto typeID = GetComponentTypeID<T>();
        return std::ranges::find(_typeIDs, typeID) != _typeIDs.end();
    }

    // Get all type IDs in sorted order
    [[nodiscard]] const std::vector<ComponentTypeID>& GetTypeIDs() const noexcept
    {
        return _typeIDs;
    }

    // Equality comparison for archetype matching
    [[nodiscard]] bool operator==(const ComponentSignature& other) const noexcept
    {
        return _typeIDs == other._typeIDs;
    }

    // Hash for use in unordered containers
    [[nodiscard]] size_t Hash() const noexcept
    {
        size_t hash = 0;
        for (const auto typeID : _typeIDs)
        {
            // Simple hash combine (boost-style)
            hash ^= typeID + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }

    // Number of components in signature
    [[nodiscard]] size_t Count() const noexcept { return _typeIDs.size(); }

    // Check if empty
    [[nodiscard]] bool Empty() const noexcept { return _typeIDs.empty(); }

private:
    std::vector<ComponentTypeID> _typeIDs;
};

} // namespace Engine::ECS

// Hash specialization for ComponentSignature to use in unordered_map
template<>
struct std::hash<Engine::ECS::ComponentSignature> {
    size_t operator()(const Engine::ECS::ComponentSignature& sig) const noexcept
    {
        return sig.Hash();
    }
};

