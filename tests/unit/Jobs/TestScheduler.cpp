// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - Job Scheduler Unit Tests
// License: MIT

#include <gtest/gtest.h>
#include "Jobs/Scheduler.hpp"
#include "Jobs/Types.hpp"
#include "Core/Platform/Time.hpp"
#include <atomic>
#include <thread>
#include <chrono>

using namespace Engine;
using namespace Engine::Jobs;

class SchedulerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize platform and time for tests
        Platform::Initialize();
        Time::Initialize();
        
        // Initialize scheduler
        Scheduler::Instance().Initialize();
    }

    void TearDown() override
    {
        // Shutdown scheduler
        Scheduler::Instance().Shutdown();
        
        // Shutdown platform
        Platform::Shutdown();
    }
};

// Test: Scheduler initialization
TEST_F(SchedulerTest, Initialization)
{
    EXPECT_TRUE(Scheduler::Instance().IsInitialized());
    EXPECT_GT(Scheduler::Instance().GetWorkerCount(), 0u);
}

// Test: Simple job submission and execution
TEST_F(SchedulerTest, SimpleJobExecution)
{
    std::atomic<bool> executed{false};

    JobDesc desc{.name = "SimpleJob"};
    auto handle = Scheduler::Instance().Submit(desc, [&executed]() {
        executed.store(true, std::memory_order_release);
    });

    EXPECT_TRUE(handle.IsValid());

    // Wait for job to complete
    Scheduler::Instance().Wait(handle);

    EXPECT_TRUE(executed.load(std::memory_order_acquire));
}

// Test: Multiple jobs execution
TEST_F(SchedulerTest, MultipleJobs)
{
    constexpr usize numJobs = 100;
    std::atomic<usize> counter{0};

    std::vector<JobHandle> handles;
    handles.reserve(numJobs);

    // Submit jobs
    for (usize i = 0; i < numJobs; ++i)
    {
        JobDesc desc{.name = "Job_" + std::to_string(i)};
        auto handle = Scheduler::Instance().Submit(desc, [&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        });
        handles.push_back(handle);
    }

    // Wait for all jobs
    for (auto handle : handles)
    {
        Scheduler::Instance().Wait(handle);
    }

    EXPECT_EQ(counter.load(std::memory_order_acquire), numJobs);
}

// Test: Job priorities (basic test - just verify they're accepted)
TEST_F(SchedulerTest, JobPriorities)
{
    std::atomic<usize> counter{0};

    JobDesc lowPriority{.name = "LowPriority", .priority = JobPriority::Low};
    JobDesc normalPriority{.name = "NormalPriority", .priority = JobPriority::Normal};
    JobDesc highPriority{.name = "HighPriority", .priority = JobPriority::High};
    JobDesc criticalPriority{.name = "CriticalPriority", .priority = JobPriority::Critical};

    auto h1 = Scheduler::Instance().Submit(lowPriority, [&counter]() { counter++; });
    auto h2 = Scheduler::Instance().Submit(normalPriority, [&counter]() { counter++; });
    auto h3 = Scheduler::Instance().Submit(highPriority, [&counter]() { counter++; });
    auto h4 = Scheduler::Instance().Submit(criticalPriority, [&counter]() { counter++; });

    Scheduler::Instance().Wait(h1);
    Scheduler::Instance().Wait(h2);
    Scheduler::Instance().Wait(h3);
    Scheduler::Instance().Wait(h4);

    EXPECT_EQ(counter.load(), 4u);
}

// Test: Job affinity (basic test - just verify they're accepted)
TEST_F(SchedulerTest, JobAffinity)
{
    std::atomic<bool> executed{false};

    JobDesc mainThreadJob{
        .name = "MainThreadJob",
        .affinity = JobAffinity::MainThread
    };

    auto handle = Scheduler::Instance().Submit(mainThreadJob, [&executed]() {
        executed.store(true);
    });

    Scheduler::Instance().Wait(handle);
    EXPECT_TRUE(executed.load());
}

// Test: ParallelFor basic functionality
TEST_F(SchedulerTest, ParallelForBasic)
{
    constexpr usize arraySize = 1000;
    std::vector<usize> data(arraySize, 0);

    // Fill array with indices using ParallelFor
    Scheduler::Instance().ParallelFor(0, arraySize, 100, [&data](usize i) {
        data[i] = i;
    });

    // Verify all elements were set correctly
    for (usize i = 0; i < arraySize; ++i)
    {
        EXPECT_EQ(data[i], i);
    }
}

// Test: ParallelFor with small grain (should execute inline)
TEST_F(SchedulerTest, ParallelForSmallGrain)
{
    constexpr usize arraySize = 10;
    std::vector<usize> data(arraySize, 0);

    // Small range should execute inline
    Scheduler::Instance().ParallelFor(0, arraySize, 100, [&data](usize i) {
        data[i] = i * 2;
    });

    for (usize i = 0; i < arraySize; ++i)
    {
        EXPECT_EQ(data[i], i * 2);
    }
}

// Test: ParallelFor with large dataset
TEST_F(SchedulerTest, ParallelForLarge)
{
    constexpr usize arraySize = 10000;
    std::vector<usize> data(arraySize, 0);

    Scheduler::Instance().ParallelFor(0, arraySize, 250, [&data](usize i) {
        data[i] = i * i;
    });

    for (usize i = 0; i < arraySize; ++i)
    {
        EXPECT_EQ(data[i], i * i);
    }
}

// Test: Job statistics
TEST_F(SchedulerTest, Statistics)
{
    auto statsBefore = Scheduler::Instance().GetStats();

    constexpr usize numJobs = 50;
    std::vector<JobHandle> handles;

    for (usize i = 0; i < numJobs; ++i)
    {
        JobDesc desc{.name = "StatsJob"};
        auto handle = Scheduler::Instance().Submit(desc, []() {
            // Simple job
        });
        handles.push_back(handle);
    }

    for (auto handle : handles)
    {
        Scheduler::Instance().Wait(handle);
    }

    auto statsAfter = Scheduler::Instance().GetStats();

    EXPECT_EQ(statsAfter.jobsSubmitted - statsBefore.jobsSubmitted, numJobs);
    EXPECT_EQ(statsAfter.jobsExecuted - statsBefore.jobsExecuted, numJobs);
}

// Test: Exception handling in jobs
TEST_F(SchedulerTest, ExceptionHandling)
{
    std::atomic<bool> jobAfterExceptionExecuted{false};

    // Submit job that throws
    JobDesc desc1{.name = "ThrowingJob"};
    auto handle1 = Scheduler::Instance().Submit(desc1, []() {
        throw std::runtime_error("Test exception");
    });

    // Submit another job after the throwing one
    JobDesc desc2{.name = "NormalJob"};
    auto handle2 = Scheduler::Instance().Submit(desc2, [&jobAfterExceptionExecuted]() {
        jobAfterExceptionExecuted.store(true);
    });

    // Wait for both jobs
    Scheduler::Instance().Wait(handle1);
    Scheduler::Instance().Wait(handle2);

    // Scheduler should still be functional
    EXPECT_TRUE(jobAfterExceptionExecuted.load());
}

// Test: Concurrent job submission from multiple threads
TEST_F(SchedulerTest, ConcurrentSubmission)
{
    constexpr usize numThreads = 4;
    constexpr usize jobsPerThread = 25;
    std::atomic<usize> counter{0};

    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    for (usize t = 0; t < numThreads; ++t)
    {
        threads.emplace_back([&counter]() {
            for (usize i = 0; i < jobsPerThread; ++i)
            {
                JobDesc desc{.name = "ConcurrentJob"};
                auto handle = Scheduler::Instance().Submit(desc, [&counter]() {
                    counter.fetch_add(1, std::memory_order_relaxed);
                });
                Scheduler::Instance().Wait(handle);
            }
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(counter.load(std::memory_order_acquire), numThreads * jobsPerThread);
}

// Test: Wait on invalid handle (should not crash)
TEST_F(SchedulerTest, WaitOnInvalidHandle)
{
    JobHandle invalidHandle;
    EXPECT_FALSE(invalidHandle.IsValid());

    // Should not crash
    EXPECT_NO_THROW(Scheduler::Instance().Wait(invalidHandle));
}

