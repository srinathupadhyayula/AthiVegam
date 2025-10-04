#pragma once
#include "Query.hpp"
#include "Jobs/Scheduler.hpp"
#include "Core/Logger.hpp"
#include <atomic>
#include <memory>

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
        {
            return;
        }

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

        // Use shared_ptr to avoid dangling reference when method returns
        // Jobs execute asynchronously, so stack variables would be invalid
        auto completedChunks = std::make_shared<std::atomic<size_t>>(0);

        // Store function in a shared_ptr to safely share across jobs
        // This avoids issues with capturing forwarding references
        auto sharedFunc = std::make_shared<std::decay_t<Func>>(std::forward<Func>(func));

        // Submit a job for each chunk
        for (const auto& [archetypeIdx, chunkIdx] : chunkIndices)
        {
            if (archetypeIdx >= archetypes.size())
                continue;

            Archetype* archetype = archetypes[archetypeIdx];
            if (!archetype)
                continue;

            const auto& chunks = archetype->GetChunks();
            if (chunkIdx >= chunks.size())
                continue;

            Chunk* chunk = chunks[chunkIdx].get();
            const size_t count = chunk ? chunk->Count() : 0;
            if (count == 0) continue;

            // Prepare column pointers now to avoid accessing vectors inside jobs
            auto columnTuple = std::make_tuple(chunk->GetColumn<Ts>()...);

            // Validate all column pointers before submitting job
            // Skip this chunk if any column pointer is null
            if (((std::get<Ts*>(columnTuple) == nullptr) || ...))
                continue;

            Jobs::JobDesc desc{
                .name = "ECS_ParallelQuery",
                .priority = Jobs::JobPriority::Normal
            };

            // Capture shared_ptr by value to ensure lifetime safety
            Jobs::Scheduler::Instance().Submit(desc, [columnTuple, count, sharedFunc, completedChunks]() mutable {
                for (size_t i = 0; i < count; ++i)
                {
                    std::apply([&](auto*... columns) {
                        (*sharedFunc)(columns[i]...);
                    }, columnTuple);
                }
                completedChunks->fetch_add(1, std::memory_order_release);
            });
        }

        // Wait for all chunks to complete
        while (completedChunks->load(std::memory_order_acquire) < totalChunks)
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

        // Use shared_ptr to avoid dangling reference when method returns
        // Jobs execute asynchronously, so stack variables would be invalid
        auto completedChunks = std::make_shared<std::atomic<size_t>>(0);

        // Store function in a shared_ptr to safely share across jobs
        auto sharedFunc = std::make_shared<std::decay_t<Func>>(std::forward<Func>(func));

        for (size_t i = 0; i < chunkIndices.size(); ++i)
        {
            const auto& [archetypeIdx, chunkIdx] = chunkIndices[i];

            if (archetypeIdx >= archetypes.size()) continue;
            Archetype* archetype = archetypes[archetypeIdx];
            if (!archetype) continue;

            const auto& chunks = archetype->GetChunks();
            if (chunkIdx >= chunks.size()) continue;

            Chunk* chunk = chunks[chunkIdx].get();
            const size_t count = chunk ? chunk->Count() : 0;
            if (count == 0) continue;

            auto columnTuple = std::make_tuple(chunk->GetColumn<Ts>()...);

            Jobs::JobDesc desc{
                .name = "ECS_ParallelQueryChunk",
                .priority = Jobs::JobPriority::Normal
            };

            // Capture shared_ptr by value to ensure lifetime safety
            Jobs::Scheduler::Instance().Submit(desc, [i, columnTuple, count, sharedFunc, completedChunks]() mutable {
                std::apply([&](auto*... columns) {
                    (*sharedFunc)(i, columns..., count);
                }, columnTuple);
                completedChunks->fetch_add(1, std::memory_order_release);
            });
        }

        while (completedChunks->load(std::memory_order_acquire) < totalChunks)
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

