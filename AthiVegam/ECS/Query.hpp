#pragma once
#include "ComponentTraits.hpp"
#include "Archetype.hpp"
#include <vector>
#include <tuple>
#include <algorithm>
#include <ranges>

namespace Engine::ECS {

// Forward declaration
class World;

// Helper to mark components for exclusion in queries
template<Component... Ts>
struct Exclude {
    static constexpr size_t Count = sizeof...(Ts);
};

// Query iterator for chunk-level iteration
template<Component... Ts>
class QueryIterator {
public:
    using ComponentTuple = std::tuple<Ts*...>;
    
    QueryIterator(const std::vector<Archetype*>& archetypes, size_t archetypeIdx, size_t chunkIdx)
        : _archetypes(archetypes)
        , _archetypeIdx(archetypeIdx)
        , _chunkIdx(chunkIdx)
    {
        SkipEmptyChunks();
    }
    
    // Dereference returns tuple of component pointers for current chunk
    ComponentTuple operator*() const noexcept
    {
        if (_archetypeIdx >= _archetypes.size())
            return ComponentTuple{};
            
        Archetype* archetype = _archetypes[_archetypeIdx];
        const auto& chunks = archetype->GetChunks();
        
        if (_chunkIdx >= chunks.size())
            return ComponentTuple{};
            
        Chunk* chunk = chunks[_chunkIdx].get();
        return std::make_tuple(chunk->GetColumn<Ts>()...);
    }
    
    // Get entity count in current chunk
    [[nodiscard]] size_t Count() const noexcept
    {
        if (_archetypeIdx >= _archetypes.size())
            return 0;
            
        Archetype* archetype = _archetypes[_archetypeIdx];
        const auto& chunks = archetype->GetChunks();
        
        if (_chunkIdx >= chunks.size())
            return 0;
            
        return chunks[_chunkIdx]->Count();
    }
    
    // Pre-increment
    QueryIterator& operator++() noexcept
    {
        ++_chunkIdx;
        SkipEmptyChunks();
        return *this;
    }
    
    // Post-increment
    QueryIterator operator++(int) noexcept
    {
        QueryIterator tmp = *this;
        ++(*this);
        return tmp;
    }
    
    // Equality comparison
    bool operator==(const QueryIterator& other) const noexcept
    {
        return _archetypeIdx == other._archetypeIdx && _chunkIdx == other._chunkIdx;
    }
    
    bool operator!=(const QueryIterator& other) const noexcept
    {
        return !(*this == other);
    }
    
private:
    void SkipEmptyChunks() noexcept
    {
        while (_archetypeIdx < _archetypes.size())
        {
            Archetype* archetype = _archetypes[_archetypeIdx];
            const auto& chunks = archetype->GetChunks();
            
            // Skip to next non-empty chunk in current archetype
            while (_chunkIdx < chunks.size() && chunks[_chunkIdx]->Count() == 0)
            {
                ++_chunkIdx;
            }
            
            // If we found a non-empty chunk, we're done
            if (_chunkIdx < chunks.size())
                return;
                
            // Move to next archetype
            ++_archetypeIdx;
            _chunkIdx = 0;
        }
    }
    
    const std::vector<Archetype*>& _archetypes;
    size_t _archetypeIdx;
    size_t _chunkIdx;
};

// Query class for iterating over entities with specific components
template<Component... Ts>
class Query {
public:
    using Iterator = QueryIterator<Ts...>;
    using ComponentTuple = typename Iterator::ComponentTuple;
    
    explicit Query(std::vector<Archetype*> matchingArchetypes)
        : _matchingArchetypes(std::move(matchingArchetypes))
    {
    }
    
    // Get iterator to first chunk
    [[nodiscard]] Iterator begin() const noexcept
    {
        return Iterator(_matchingArchetypes, 0, 0);
    }
    
    // Get iterator to end
    [[nodiscard]] Iterator end() const noexcept
    {
        return Iterator(_matchingArchetypes, _matchingArchetypes.size(), 0);
    }
    
    // Get total number of chunks across all matching archetypes
    [[nodiscard]] size_t ChunkCount() const noexcept
    {
        size_t count = 0;
        for (const auto* archetype : _matchingArchetypes)
        {
            count += archetype->GetChunks().size();
        }
        return count;
    }
    
    // Get total entity count across all matching archetypes
    [[nodiscard]] size_t EntityCount() const noexcept
    {
        size_t count = 0;
        for (const auto* archetype : _matchingArchetypes)
        {
            for (const auto& chunk : archetype->GetChunks())
            {
                count += chunk->Count();
            }
        }
        return count;
    }
    
    // Check if query has any matching entities
    [[nodiscard]] bool Empty() const noexcept
    {
        return EntityCount() == 0;
    }
    
    // Execute a function for each entity in the query
    // Function signature: void(Ts&... components)
    template<typename Func>
    void ForEach(Func&& func) const
    {
        for (auto it = begin(); it != end(); ++it)
        {
            auto [columns...] = *it;
            const size_t count = it.Count();
            
            for (size_t i = 0; i < count; ++i)
            {
                func(columns[i]...);
            }
        }
    }
    
private:
    std::vector<Archetype*> _matchingArchetypes;
};

// Helper to check if a signature matches query requirements
template<Component... IncludeTs>
inline bool SignatureMatches(const ComponentSignature& signature)
{
    // Check that all required components are present
    return (signature.Contains<IncludeTs>() && ...);
}

template<Component... IncludeTs, Component... ExcludeTs>
inline bool SignatureMatches(const ComponentSignature& signature, Exclude<ExcludeTs...>)
{
    // Check that all required components are present
    bool hasAllRequired = (signature.Contains<IncludeTs>() && ...);
    
    // Check that none of the excluded components are present
    bool hasAnyExcluded = (signature.Contains<ExcludeTs>() || ...);
    
    return hasAllRequired && !hasAnyExcluded;
}

} // namespace Engine::ECS

