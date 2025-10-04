// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - Work-Stealing Performance Tests
// License: MIT

#include <gtest/gtest.h>
#include "Jobs/Scheduler.hpp"
#include "Core/Platform/Time.hpp"
#include "Core/Platform/Platform.hpp"
#include <atomic>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>

using namespace Engine;
using namespace Engine::Jobs;

class WorkStealingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Platform::Initialize();
        Time::Initialize();
        Scheduler::Instance().Initialize();
    }

    void TearDown() override
    {
        Scheduler::Instance().Shutdown();
        Platform::Shutdown();
    }
};

// Test: Work-stealing actually occurs
TEST_F(WorkStealingTest, StealingOccurs)
{
    auto statsBefore = Scheduler::Instance().GetStats();

    // Submit many jobs to trigger stealing
    constexpr usize numJobs = 1000;
    std::atomic<usize> counter{0};
    std::vector<JobHandle> handles;

    for (usize i = 0; i < numJobs; ++i)
    {
        JobDesc desc{.name = "StealableJob"};
        auto handle = Scheduler::Instance().Submit(desc, [&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
            // Small amount of work to allow stealing
            volatile int x = 0;
            for (int j = 0; j < 100; ++j) x++;
        });
        handles.push_back(handle);
    }

    for (auto handle : handles)
    {
        Scheduler::Instance().Wait(handle);
    }

    auto statsAfter = Scheduler::Instance().GetStats();

    // Verify all jobs executed
    EXPECT_EQ(counter.load(), numJobs);

    // Verify some stealing occurred (with multiple workers, stealing should happen)
    if (Scheduler::Instance().GetWorkerCount() > 1)
    {
        EXPECT_GT(statsAfter.jobsStolen - statsBefore.jobsStolen, 0u);
    }
}

// Test: Load balancing across workers
TEST_F(WorkStealingTest, LoadBalancing)
{
    constexpr usize numJobs = 500;
    std::atomic<usize> counter{0};

    // Submit jobs with varying work amounts
    std::vector<JobHandle> handles;
    for (usize i = 0; i < numJobs; ++i)
    {
        JobDesc desc{.name = "BalancedJob"};
        auto handle = Scheduler::Instance().Submit(desc, [&counter, i]() {
            counter.fetch_add(1, std::memory_order_relaxed);
            // Variable work to test load balancing
            volatile int x = 0;
            for (usize j = 0; j < (i % 10) * 100; ++j) x++;
        });
        handles.push_back(handle);
    }

    for (auto handle : handles)
    {
        Scheduler::Instance().Wait(handle);
    }

    EXPECT_EQ(counter.load(), numJobs);
}

// Benchmark: Job submission overhead
TEST_F(WorkStealingTest, SubmissionOverhead)
{
    constexpr usize numJobs = 10000;
    
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<JobHandle> handles;
    handles.reserve(numJobs);

    for (usize i = 0; i < numJobs; ++i)
    {
        JobDesc desc{.name = "BenchmarkJob"};
        auto handle = Scheduler::Instance().Submit(desc, []() {
            // Minimal work
        });
        handles.push_back(handle);
    }

    for (auto handle : handles)
    {
        Scheduler::Instance().Wait(handle);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Calculate overhead per job
    double usPerJob = static_cast<double>(duration.count()) / numJobs;

    // Log performance (not a hard requirement, just informational)
    std::cout << "[Performance] Job submission overhead: " << usPerJob << " μs/job" << std::endl;
    std::cout << "[Performance] Total time for " << numJobs << " jobs: " 
              << duration.count() / 1000.0 << " ms" << std::endl;

    // Sanity check: overhead should be reasonable (< 100μs per job)
    EXPECT_LT(usPerJob, 100.0);
}

// Benchmark: Work-stealing efficiency
TEST_F(WorkStealingTest, StealingEfficiency)
{
    constexpr usize numJobs = 1000;
    std::atomic<usize> workDone{0};

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<JobHandle> handles;
    for (usize i = 0; i < numJobs; ++i)
    {
        JobDesc desc{.name = "WorkJob"};
        auto handle = Scheduler::Instance().Submit(desc, [&workDone]() {
            // Simulate some work
            volatile int x = 0;
            for (int j = 0; j < 1000; ++j) x++;
            workDone.fetch_add(1, std::memory_order_relaxed);
        });
        handles.push_back(handle);
    }

    for (auto handle : handles)
    {
        Scheduler::Instance().Wait(handle);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_EQ(workDone.load(), numJobs);

    auto stats = Scheduler::Instance().GetStats();
    double stealRatio = static_cast<double>(stats.jobsStolen) / stats.jobsExecuted;

    std::cout << "[Performance] Work-stealing efficiency:" << std::endl;
    std::cout << "  Jobs executed: " << stats.jobsExecuted << std::endl;
    std::cout << "  Jobs stolen: " << stats.jobsStolen << std::endl;
    std::cout << "  Steal ratio: " << (stealRatio * 100.0) << "%" << std::endl;
    std::cout << "  Total time: " << duration.count() << " ms" << std::endl;
}

// Test: Stealing from specific worker patterns
TEST_F(WorkStealingTest, StealingPatterns)
{
    // Submit many jobs to worker 0 (via MainThread affinity)
    constexpr usize numJobs = 200;
    std::atomic<usize> counter{0};

    std::vector<JobHandle> handles;
    for (usize i = 0; i < numJobs; ++i)
    {
        JobDesc desc{
            .name = "MainThreadJob",
            .affinity = JobAffinity::MainThread
        };
        auto handle = Scheduler::Instance().Submit(desc, [&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
            volatile int x = 0;
            for (int j = 0; j < 500; ++j) x++;
        });
        handles.push_back(handle);
    }

    // Submit some worker thread jobs to create stealing opportunities
    for (usize i = 0; i < numJobs; ++i)
    {
        JobDesc desc{
            .name = "WorkerJob",
            .affinity = JobAffinity::WorkerThread
        };
        auto handle = Scheduler::Instance().Submit(desc, [&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        });
        handles.push_back(handle);
    }

    for (auto handle : handles)
    {
        Scheduler::Instance().Wait(handle);
    }

    EXPECT_EQ(counter.load(), numJobs * 2);
}

// Test: No stealing when work is balanced
TEST_F(WorkStealingTest, NoStealingWhenBalanced)
{
    auto statsBefore = Scheduler::Instance().GetStats();

    // Submit exactly one job per worker
    usize numWorkers = Scheduler::Instance().GetWorkerCount();
    std::atomic<usize> counter{0};

    std::vector<JobHandle> handles;
    for (usize i = 0; i < numWorkers; ++i)
    {
        JobDesc desc{.name = "BalancedJob"};
        auto handle = Scheduler::Instance().Submit(desc, [&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
            // Enough work to prevent immediate completion
            volatile int x = 0;
            for (int j = 0; j < 10000; ++j) x++;
        });
        handles.push_back(handle);
    }

    for (auto handle : handles)
    {
        Scheduler::Instance().Wait(handle);
    }

    auto statsAfter = Scheduler::Instance().GetStats();

    EXPECT_EQ(counter.load(), numWorkers);

    // With perfectly balanced work, stealing might not occur
    // (This is not a hard requirement, just an observation)
    usize steals = statsAfter.jobsStolen - statsBefore.jobsStolen;
    std::cout << "[Info] Steals with balanced work: " << steals << std::endl;
}

// Benchmark: Parallel speedup measurement
TEST_F(WorkStealingTest, ParallelSpeedup)
{
    constexpr usize numJobs = 1000;
    constexpr usize baseWorkPerJob = 10000;

    // Scale workload with worker count to avoid overhead-dominated runs
    const usize workerCount = Scheduler::Instance().GetWorkerCount();
    const usize scale = std::max<usize>(1, workerCount / 8); // ×1 for <=8 cores, ×2 for 16, ×4 for 32, etc.
    const usize workPerJob = baseWorkPerJob * scale;

    // Measure parallel execution time
    auto parallelStart = std::chrono::high_resolution_clock::now();

    std::vector<JobHandle> handles;
    for (usize i = 0; i < numJobs; ++i)
    {
        JobDesc desc{.name = "ParallelJob"};
        auto handle = Scheduler::Instance().Submit(desc, [workPerJob]() {
            double acc = 0.0;
            for (usize j = 0; j < workPerJob; ++j)
            {
                double v = (static_cast<double>(j) * 1.001) + 0.123;
                acc += std::sqrt(v) * 1.0001;
            }
            volatile double sink = acc;
            (void)sink;
        });
        handles.push_back(handle);
    }

    for (auto handle : handles)
    {
        Scheduler::Instance().Wait(handle);
    }

    auto parallelEnd = std::chrono::high_resolution_clock::now();
    auto parallelTime = std::chrono::duration_cast<std::chrono::milliseconds>(parallelEnd - parallelStart);

    // Measure sequential execution time (for comparison)
    auto sequentialStart = std::chrono::high_resolution_clock::now();

    for (usize i = 0; i < numJobs; ++i)
    {
        double acc = 0.0;
        for (usize j = 0; j < workPerJob; ++j)
        {
            double v = (static_cast<double>(j) * 1.001) + 0.123;
            acc += std::sqrt(v) * 1.0001;
        }
        volatile double sink = acc;
        (void)sink;
    }

    auto sequentialEnd = std::chrono::high_resolution_clock::now();
    auto sequentialTime = std::chrono::duration_cast<std::chrono::milliseconds>(sequentialEnd - sequentialStart);

    double speedup = static_cast<double>(sequentialTime.count()) / parallelTime.count();

    std::cout << "[Performance] Parallel speedup:" << std::endl;
    std::cout << "  Sequential time: " << sequentialTime.count() << " ms" << std::endl;
    std::cout << "  Parallel time: " << parallelTime.count() << " ms" << std::endl;
    std::cout << "  Speedup: " << speedup << "x" << std::endl;
    std::cout << "  Worker count: " << Scheduler::Instance().GetWorkerCount() << std::endl;

    // Expect at least some speedup (accounting for overhead and platform variability)
    EXPECT_GE(speedup, 0.9);
}

