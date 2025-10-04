#pragma once
#include "Query.hpp"
#include "Jobs/Scheduler.hpp"
#include <atomic>

namespace Engine::ECS {

// Parallel query execution using Jobs system
// This provides thread-safe parallel iteration over query results
template<Component... Ts>
class ParallelQueryExecutor {
public:
    explicit ParallelQueryExecutor(const Query<Ts...>& query)
        : _query(query)
    {
    }
    
    // Execute function for each entity in parallel
    // Function signature: void(Ts&... components)
    template<typename Func>
    void Execute(Func&& func) const
    {
        // Get all non-empty chunks
        auto chunkIndices = _query.GetChunkIndices();

        if (chunkIndices.empty())
            return;

        // Check if Jobs system is initialized
        if (!Jobs::Scheduler::Instance().IsInitialized())
        {
            // Fallback to sequential execution
            _query.ForEach(std::forward<Func>(func));
            return;
        }

        // Cache archetype pointers to avoid accessing query during parallel execution
        const auto& archetypes = _query.GetMatchingArchetypes();

        // Parallel execution using Jobs system
        const size_t totalChunks = chunkIndices.size();
        std::atomic<size_t> completedChunks{0};

        // Submit a job for each chunk
        for (const auto& [archetypeIdx, chunkIdx] : chunkIndices)
        {
            Jobs::JobDesc desc{
                .name = "ECS_ParallelQuery",
                .priority = Jobs::JobPriority::Normal
            };

            // Capture archetype pointer directly to avoid index lookup in job
            Archetype* archetype = archetypes[archetypeIdx];

            // Capture chunk processing in a job
            Jobs::Scheduler::Instance().Submit(desc, [archetype, chunkIdx, func, &completedChunks]() {
                ProcessChunkDirect(archetype, chunkIdx, func);
                completedChunks.fetch_add(1, std::memory_order_release);
            });
        }

        // Wait for all chunks to complete
        while (completedChunks.load(std::memory_order_acquire) < totalChunks)
        {
            Engine::Threading::YieldThread();
        }
    }
    
    // Execute function for each chunk in parallel
    // Function signature: void(size_t chunkIndex, Ts*... componentColumns, size_t entityCount)
    template<typename Func>
    void ExecuteChunks(Func&& func) const
    {
        auto chunkIndices = _query.GetChunkIndices();

        if (chunkIndices.empty())
            return;

        // Cache archetype pointers
        const auto& archetypes = _query.GetMatchingArchetypes();

        if (!Jobs::Scheduler::Instance().IsInitialized())
        {
            // Fallback: execute sequentially
            for (size_t i = 0; i < chunkIndices.size(); ++i)
            {
                const auto& [archetypeIdx, chunkIdx] = chunkIndices[i];
                Archetype* archetype = archetypes[archetypeIdx];
                ProcessChunkWithIndex(i, archetype, chunkIdx, func);
            }
            return;
        }

        const size_t totalChunks = chunkIndices.size();
        std::atomic<size_t> completedChunks{0};

        for (size_t i = 0; i < chunkIndices.size(); ++i)
        {
            const auto& [archetypeIdx, chunkIdx] = chunkIndices[i];

            Jobs::JobDesc desc{
                .name = "ECS_ParallelQueryChunk",
                .priority = Jobs::JobPriority::Normal
            };

            // Capture archetype pointer directly
            Archetype* archetype = archetypes[archetypeIdx];

            Jobs::Scheduler::Instance().Submit(desc, [i, archetype, chunkIdx, func, &completedChunks]() {
                ProcessChunkWithIndex(i, archetype, chunkIdx, func);
                completedChunks.fetch_add(1, std::memory_order_release);
            });
        }

        while (completedChunks.load(std::memory_order_acquire) < totalChunks)
        {
            Engine::Threading::YieldThread();
        }
    }
    
private:
    // Process chunk entity-by-entity (for Execute)
    // Calls func(component1, component2, ...) for each entity
    template<typename Func>
    static void ProcessChunkDirect(Archetype* archetype, size_t chunkIdx, Func func)
    {
        if (!archetype)
            return;

        const auto& chunks = archetype->GetChunks();

        if (chunkIdx >= chunks.size())
            return;

        Chunk* chunk = chunks[chunkIdx].get();
        const size_t count = chunk->Count();

        if (count == 0)
            return;

        // Get component columns
        auto columnTuple = std::make_tuple(chunk->GetColumn<Ts>()...);

        // Process each entity in the chunk
        for (size_t i = 0; i < count; ++i)
        {
            std::apply([&func, i](auto*... columns) {
                func(columns[i]...);
            }, columnTuple);
        }
    }

    // Process entire chunk at once (for ExecuteChunks)
    // Calls func(chunkIndex, column1*, column2*, ..., entityCount) once per chunk
    template<typename Func>
    static void ProcessChunkWithIndex(size_t chunkIndex, Archetype* archetype, size_t chunkIdx, Func func)
    {
        if (!archetype)
            return;

        const auto& chunks = archetype->GetChunks();

        if (chunkIdx >= chunks.size())
            return;

        Chunk* chunk = chunks[chunkIdx].get();
        const size_t count = chunk->Count();

        if (count == 0)
            return;

        // Get component columns
        auto columnTuple = std::make_tuple(chunk->GetColumn<Ts>()...);

        // Call user function with chunk data
        std::apply([&func, chunkIndex, count](auto*... columns) {
            func(chunkIndex, columns..., count);
        }, columnTuple);
    }
    
    const std::vector<Archetype*>& GetMatchingArchetypes() const
    {
        return _query.GetMatchingArchetypes();
    }
    
    const Query<Ts...>& _query;
};

// Helper function to create parallel query executor
template<Component... Ts>
inline ParallelQueryExecutor<Ts...> MakeParallel(const Query<Ts...>& query)
{
    return ParallelQueryExecutor<Ts...>(query);
}

} // namespace Engine::ECS

