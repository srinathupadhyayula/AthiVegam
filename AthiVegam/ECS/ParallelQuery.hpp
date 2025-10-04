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
            
            // Capture chunk processing in a job
            Jobs::Scheduler::Instance().Submit(desc, [this, archetypeIdx, chunkIdx, func, &completedChunks]() {
                ProcessChunk(archetypeIdx, chunkIdx, func);
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
        
        if (!Jobs::Scheduler::Instance().IsInitialized())
        {
            // Fallback: execute sequentially
            for (size_t i = 0; i < chunkIndices.size(); ++i)
            {
                const auto& [archetypeIdx, chunkIdx] = chunkIndices[i];
                ProcessChunkDirect(i, archetypeIdx, chunkIdx, func);
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
            
            Jobs::Scheduler::Instance().Submit(desc, [this, i, archetypeIdx, chunkIdx, func, &completedChunks]() {
                ProcessChunkDirect(i, archetypeIdx, chunkIdx, func);
                completedChunks.fetch_add(1, std::memory_order_release);
            });
        }
        
        while (completedChunks.load(std::memory_order_acquire) < totalChunks)
        {
            Engine::Threading::YieldThread();
        }
    }
    
private:
    template<typename Func>
    void ProcessChunk(size_t archetypeIdx, size_t chunkIdx, Func func) const
    {
        // Get archetype and chunk
        const auto& archetypes = GetMatchingArchetypes();
        if (archetypeIdx >= archetypes.size())
            return;
            
        Archetype* archetype = archetypes[archetypeIdx];
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
    
    template<typename Func>
    void ProcessChunkDirect(size_t chunkIndex, size_t archetypeIdx, size_t chunkIdx, Func func) const
    {
        const auto& archetypes = GetMatchingArchetypes();
        if (archetypeIdx >= archetypes.size())
            return;
            
        Archetype* archetype = archetypes[archetypeIdx];
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

