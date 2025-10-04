#pragma once
#include "ComponentTraits.hpp"
#include "Archetype.hpp"
#include <vector>
#include <tuple>
#include <algorithm>
#include <ranges>
#include <utility>
#include <atomic>

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
    
    // Execute a function for each entity in the query (sequential)
    // Function signature: void(Ts&... components)
    template<typename Func>
    void ForEach(Func&& func) const
    {
        for (auto it = begin(); it != end(); ++it)
        {
            auto columnTuple = *it;
            const size_t count = it.Count();

            // Apply function to each entity in the chunk
            ForEachInChunk(std::forward<Func>(func), columnTuple, count, std::index_sequence_for<Ts...>{});
        }
    }

    // Execute a function for each entity in the query (parallel)
    // Function signature: void(Ts&... components)
    // Uses Jobs system for chunk-level parallelism
    template<typename Func>
    void ForEachParallel(Func&& func) const;

    // Get all chunks for parallel processing
    // Returns vector of (archetype_index, chunk_index) pairs
    [[nodiscard]] std::vector<std::pair<size_t, size_t>> GetChunkIndices() const noexcept
    {
        std::vector<std::pair<size_t, size_t>> indices;

        for (size_t archetypeIdx = 0; archetypeIdx < _matchingArchetypes.size(); ++archetypeIdx)
        {
            const auto& chunks = _matchingArchetypes[archetypeIdx]->GetChunks();
            for (size_t chunkIdx = 0; chunkIdx < chunks.size(); ++chunkIdx)
            {
                if (chunks[chunkIdx]->Count() > 0)
                {
                    indices.emplace_back(archetypeIdx, chunkIdx);
                }
            }
        }

        return indices;
    }

    // Get matching archetypes (for parallel query executor)
    [[nodiscard]] const std::vector<Archetype*>& GetMatchingArchetypes() const noexcept
    {
        return _matchingArchetypes;
    }

    // Helper to iterate over entities in a chunk
    template<typename Func, size_t... Is>
    void ForEachInChunk(Func&& func, const ComponentTuple& columns, size_t count, std::index_sequence<Is...>) const
    {
        for (size_t i = 0; i < count; ++i)
        {
            func(std::get<Is>(columns)[i]...);
        }
    }

    std::vector<Archetype*> _matchingArchetypes;
};

// ForEachParallel implementation (requires Jobs system)
template<Component... Ts>
template<typename Func>
inline void Query<Ts...>::ForEachParallel(Func&& func) const
{
    // Get all non-empty chunks
    auto chunkIndices = GetChunkIndices();

    if (chunkIndices.empty())
        return;

    // Use Jobs system for parallel processing
    // Note: This requires Jobs::Scheduler to be initialized
    // Each chunk is processed by a separate job

    std::atomic<size_t> completedChunks{0};
    const size_t totalChunks = chunkIndices.size();

    for (const auto& [archetypeIdx, chunkIdx] : chunkIndices)
    {
        Archetype* archetype = _matchingArchetypes[archetypeIdx];
        const auto& chunks = archetype->GetChunks();
        Chunk* chunk = chunks[chunkIdx].get();

        // Get component columns for this chunk
        auto columnTuple = std::make_tuple(chunk->GetColumn<Ts>()...);
        const size_t count = chunk->Count();

        // Process this chunk in parallel
        // Note: We capture by value to avoid dangling references
        auto chunkFunc = [columnTuple, count, func, &completedChunks]() {
            // Apply function to each entity in the chunk
            for (size_t i = 0; i < count; ++i)
            {
                std::apply([&func, i](auto*... columns) {
                    func(columns[i]...);
                }, columnTuple);
            }
            completedChunks.fetch_add(1, std::memory_order_release);
        };

        // Submit job (implementation will be added when integrating with Jobs)
        // For now, execute sequentially as fallback
        chunkFunc();
    }

    // Wait for all chunks to complete
    while (completedChunks.load(std::memory_order_acquire) < totalChunks)
    {
        // Yield to avoid busy-waiting
        // Note: This will be replaced with proper Jobs::Scheduler wait
    }
}

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

