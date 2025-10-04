// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - Job Scheduler
// License: MIT

#pragma once

#include "Jobs/Types.hpp"
#include "Jobs/HazardTracker.hpp"
#include "Core/Platform/Threading.hpp"
#include <atomic>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <memory>

// Import Threading types into Jobs namespace for convenience
namespace Engine::Jobs
{
    using Threading::ThreadHandle;
    using Threading::ThreadPriority;
}

namespace Engine::Jobs
{

/// @brief Work-stealing job scheduler
/// @details Manages worker threads and distributes jobs across cores.
///          Uses work-stealing for load balancing and hazard tracking for safety.
/// @example
/// @code
/// // Initialize scheduler
/// Scheduler::Instance().Initialize();
///
/// // Submit a job
/// JobDesc desc{.name = "MyJob"};
/// auto handle = Scheduler::Instance().Submit(desc, []() {
///     // Job code here
/// });
///
/// // Wait for completion
/// Scheduler::Instance().Wait(handle);
///
/// // Shutdown
/// Scheduler::Instance().Shutdown();
/// @endcode
class Scheduler
{
public:
    /// @brief Get the singleton instance
    static Scheduler& Instance();

    /// @brief Initialize the scheduler
    /// @details Creates worker threads (one per logical core)
    void Initialize();

    /// @brief Shutdown the scheduler
    /// @details Waits for pending jobs and terminates worker threads
    void Shutdown();

    /// @brief Submit a job for execution
    /// @param desc Job descriptor with metadata
    /// @param fn Job function to execute
    /// @return Handle to the submitted job
    JobHandle Submit(const JobDesc& desc, JobFunction fn);

    /// @brief Wait for a job to complete
    /// @param handle Handle to the job
    void Wait(JobHandle handle);

    /// @brief Parallel for loop
    /// @param begin Start index (inclusive)
    /// @param end End index (exclusive)
    /// @param grain Minimum chunk size per job
    /// @param fn Function to execute for each index
    template<typename Fn>
    void ParallelFor(usize begin, usize end, usize grain, Fn fn);

    /// @brief Check if scheduler is initialized
    bool IsInitialized() const { return _initialized; }

    /// @brief Get number of worker threads
    usize GetWorkerCount() const { return _workerCount; }

    /// @brief Get statistics
    struct Stats
    {
        u64 jobsSubmitted = 0;
        u64 jobsExecuted = 0;
        u64 jobsStolen = 0;
        u64 jobsCancelled = 0;
    };
    Stats GetStats() const;

private:
    Scheduler() = default;
    ~Scheduler() = default;
    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    /// @brief Internal job structure
    struct Job
    {
        JobDesc desc;
        JobFunction fn;
        JobHandle handle;
        std::atomic<JobStatus> status{JobStatus::Pending};
        std::atomic<u32> refCount{1}; // For parent-child relationships
    };

    /// @brief Per-worker thread data
    struct WorkerThread
    {
        ThreadHandle thread;
        std::deque<std::shared_ptr<Job>> queue;
        std::mutex queueMutex;
        u32 workerId;
        bool shouldExit = false;
    };

    /// @brief Worker thread main loop
    void WorkerMain(u32 workerId);

    /// @brief Try to pop a job from local queue
    std::shared_ptr<Job> PopLocal(u32 workerId);

    /// @brief Try to steal a job from another worker
    std::shared_ptr<Job> StealJob(u32 thiefId);

    /// @brief Execute a job (with hazard checking)
    void ExecuteJob(std::shared_ptr<Job> job);

    /// @brief Execute a job directly (assumes resources already acquired)
    void ExecuteJobDirect(std::shared_ptr<Job> job);

    /// @brief Try to execute deferred jobs
    void TryExecuteDeferredJobs();

    /// @brief Notify waiters that a job completed
    void NotifyJobComplete(JobHandle handle);

    /// @brief Get job by handle
    std::shared_ptr<Job> GetJob(JobHandle handle);

    // Member variables
    bool _initialized = false;
    usize _workerCount = 0;
    std::vector<std::unique_ptr<WorkerThread>> _workers;

    // Job tracking
    std::unordered_map<u64, std::shared_ptr<Job>> _jobs;
    std::mutex _jobsMutex;
    u32 _nextJobIndex = 0;
    u32 _jobVersion = 1;
    std::atomic<u32> _nextWorker{0};  // Round-robin worker selection

    // Hazard tracking
    HazardTracker _hazardTracker;
    std::deque<std::shared_ptr<Job>> _deferredJobs;
    std::mutex _deferredMutex;

    // Completion notification
    std::condition_variable _completionCV;
    std::mutex _completionMutex;

    // Statistics
    mutable std::mutex _statsMutex;
    Stats _stats;
};

// Template implementation
template<typename Fn>
void Scheduler::ParallelFor(usize begin, usize end, usize grain, Fn fn)
{
    if (begin >= end)
        return;

    const usize range = end - begin;
    
    // If range is smaller than grain, execute inline
    if (range <= grain)
    {
        for (usize i = begin; i < end; ++i)
        {
            fn(i);
        }
        return;
    }

    // Calculate number of chunks
    const usize numChunks = (range + grain - 1) / grain;
    std::atomic<usize> completedChunks{0};

    // Submit jobs for each chunk
    for (usize chunk = 0; chunk < numChunks; ++chunk)
    {
        const usize chunkBegin = begin + chunk * grain;
        const usize chunkEnd = (chunk == numChunks - 1) ? end : chunkBegin + grain;

        JobDesc desc{
            .name = "ParallelFor",
            .priority = JobPriority::Normal
        };

        // Capture fn by value to avoid dangling reference if caller scope exits
        Submit(desc, [chunkBegin, chunkEnd, fn, &completedChunks]() {
            for (usize i = chunkBegin; i < chunkEnd; ++i)
            {
                fn(i);
            }
            completedChunks.fetch_add(1, std::memory_order_release);
        });
    }

    // Wait for all chunks to complete
    while (completedChunks.load(std::memory_order_acquire) < numChunks)
    {
        // Yield to avoid busy-waiting
        Threading::YieldThread();
    }
}

} // namespace Engine::Jobs

