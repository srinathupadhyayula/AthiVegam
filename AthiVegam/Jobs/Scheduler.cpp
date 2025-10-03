// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - Job Scheduler Implementation
// License: MIT

#include "Jobs/Scheduler.hpp"
#include "Core/Platform/Platform.hpp"
#include "Core/Logger.hpp"
#include <algorithm>
#include <random>

namespace Engine::Jobs
{

Scheduler& Scheduler::Instance()
{
    static Scheduler instance;
    return instance;
}

void Scheduler::Initialize()
{
    if (_initialized)
    {
        LOG_WARN("Scheduler already initialized");
        return;
    }

    LOG_INFO("Initializing Job Scheduler...");

    // Get number of logical cores
    _workerCount = Platform::GetLogicalCoreCount();
    if (_workerCount == 0)
    {
        LOG_WARN("Failed to detect CPU cores, defaulting to 4 workers");
        _workerCount = 4;
    }

    LOG_INFO("Creating {} worker threads", _workerCount);

    // Create worker threads
    _workers.reserve(_workerCount);
    for (usize i = 0; i < _workerCount; ++i)
    {
        auto worker = std::make_unique<WorkerThread>();
        worker->workerId = static_cast<u32>(i);
        worker->shouldExit = false;

        // Create thread
        worker->thread = Threading::CreateThread(
            [this, workerId = worker->workerId]() {
                // Set thread name for debugging
                Threading::SetCurrentThreadName(("Worker_" + std::to_string(workerId)).c_str());
                WorkerMain(workerId);
            },
            ThreadPriority::Normal
        );

        _workers.push_back(std::move(worker));
    }

    _initialized = true;
    LOG_INFO("Job Scheduler initialized with {} workers", _workerCount);
}

void Scheduler::Shutdown()
{
    if (!_initialized)
    {
        return;
    }

    LOG_INFO("Shutting down Job Scheduler...");

    // Signal all workers to exit
    for (auto& worker : _workers)
    {
        std::lock_guard<std::mutex> lock(worker->queueMutex);
        worker->shouldExit = true;
    }

    // Wake up all workers
    _completionCV.notify_all();

    // Wait for all workers to finish
    for (auto& worker : _workers)
    {
        if (worker->thread)
        {
            Threading::JoinThread(worker->thread);
        }
    }

    _workers.clear();
    _jobs.clear();
    _initialized = false;

    LOG_INFO("Job Scheduler shut down. Stats: {} submitted, {} executed, {} stolen, {} cancelled",
        _stats.jobsSubmitted, _stats.jobsExecuted, _stats.jobsStolen, _stats.jobsCancelled);
}

JobHandle Scheduler::Submit(const JobDesc& desc, JobFunction fn)
{
    if (!_initialized)
    {
        LOG_ERROR("Cannot submit job: Scheduler not initialized");
        return JobHandle();
    }

    // Create job
    auto job = std::make_shared<Job>();
    job->desc = desc;
    job->fn = std::move(fn);
    
    // Generate handle
    {
        std::lock_guard<std::mutex> lock(_jobsMutex);
        job->handle = JobHandle(_nextJobIndex++, _jobVersion);
        _jobs[job->handle.Value()] = job;
    }

    // Update stats
    {
        std::lock_guard<std::mutex> lock(_statsMutex);
        _stats.jobsSubmitted++;
    }

    // Select worker based on affinity
    u32 targetWorker = 0;
    if (desc.affinity == JobAffinity::MainThread)
    {
        targetWorker = 0; // Main thread is worker 0
    }
    else
    {
        // Round-robin distribution
        static std::atomic<u32> nextWorker{0};
        targetWorker = nextWorker.fetch_add(1, std::memory_order_relaxed) % _workerCount;
    }

    // Enqueue job
    {
        std::lock_guard<std::mutex> lock(_workers[targetWorker]->queueMutex);
        _workers[targetWorker]->queue.push_back(job);
    }

    // Wake up worker
    _completionCV.notify_one();

    return job->handle;
}

void Scheduler::Wait(JobHandle handle)
{
    if (!handle.IsValid())
    {
        return;
    }

    // Spin-wait with yield for job completion
    while (true)
    {
        auto job = GetJob(handle);
        if (!job || job->status.load(std::memory_order_acquire) == JobStatus::Completed ||
            job->status.load(std::memory_order_acquire) == JobStatus::Cancelled)
        {
            break;
        }

        // Yield to avoid busy-waiting
        Threading::YieldThread();
    }
}

Scheduler::Stats Scheduler::GetStats() const
{
    std::lock_guard<std::mutex> lock(_statsMutex);
    return _stats;
}

void Scheduler::WorkerMain(u32 workerId)
{
    LOG_INFO("Worker {} started", workerId);

    while (true)
    {
        // Check exit condition
        {
            std::lock_guard<std::mutex> lock(_workers[workerId]->queueMutex);
            if (_workers[workerId]->shouldExit && _workers[workerId]->queue.empty())
            {
                break;
            }
        }

        // Try to get a job from local queue
        auto job = PopLocal(workerId);

        // If no local job, try to steal from another worker
        if (!job)
        {
            job = StealJob(workerId);
        }

        // If we got a job, execute it
        if (job)
        {
            ExecuteJob(job);
        }
        else
        {
            // No work available, yield
            Threading::YieldThread();
        }
    }

    LOG_INFO("Worker {} exiting", workerId);
}

std::shared_ptr<Scheduler::Job> Scheduler::PopLocal(u32 workerId)
{
    std::lock_guard<std::mutex> lock(_workers[workerId]->queueMutex);
    
    if (_workers[workerId]->queue.empty())
    {
        return nullptr;
    }

    auto job = _workers[workerId]->queue.front();
    _workers[workerId]->queue.pop_front();
    return job;
}

std::shared_ptr<Scheduler::Job> Scheduler::StealJob(u32 thiefId)
{
    // Try to steal from a random worker
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<u32> dist(0, static_cast<u32>(_workerCount - 1));

    for (usize attempt = 0; attempt < _workerCount; ++attempt)
    {
        u32 victimId = dist(rng);
        
        // Don't steal from ourselves
        if (victimId == thiefId)
        {
            continue;
        }

        std::lock_guard<std::mutex> lock(_workers[victimId]->queueMutex);
        
        if (!_workers[victimId]->queue.empty())
        {
            // Steal from back of queue (LIFO for better cache locality)
            auto job = _workers[victimId]->queue.back();
            _workers[victimId]->queue.pop_back();

            // Update stats
            {
                std::lock_guard<std::mutex> statsLock(_statsMutex);
                _stats.jobsStolen++;
            }

            return job;
        }
    }

    return nullptr;
}

void Scheduler::ExecuteJob(std::shared_ptr<Job> job)
{
    // Mark as running
    job->status.store(JobStatus::Running, std::memory_order_release);

    // Execute job function
    try
    {
        job->fn();
        job->status.store(JobStatus::Completed, std::memory_order_release);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Job '{}' threw exception: {}", job->desc.name, e.what());
        job->status.store(JobStatus::Completed, std::memory_order_release);
    }
    catch (...)
    {
        LOG_ERROR("Job '{}' threw unknown exception", job->desc.name);
        job->status.store(JobStatus::Completed, std::memory_order_release);
    }

    // Update stats
    {
        std::lock_guard<std::mutex> lock(_statsMutex);
        _stats.jobsExecuted++;
    }

    // Notify waiters
    NotifyJobComplete(job->handle);
}

void Scheduler::NotifyJobComplete(JobHandle handle)
{
    _completionCV.notify_all();
}

std::shared_ptr<Scheduler::Job> Scheduler::GetJob(JobHandle handle)
{
    std::lock_guard<std::mutex> lock(_jobsMutex);
    auto it = _jobs.find(handle.Value());
    return (it != _jobs.end()) ? it->second : nullptr;
}

} // namespace Engine::Jobs

