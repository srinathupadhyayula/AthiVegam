// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - Scheduler + HazardTracker Integration Tests
// License: MIT

#include <gtest/gtest.h>
#include "Jobs/Scheduler.hpp"
#include "Core/Platform/Platform.hpp"
#include "Core/Platform/Time.hpp"
#include <atomic>
#include <vector>
#include <thread>

using namespace Engine;
using namespace Engine::Jobs;

class SchedulerHazardIntegrationTest : public ::testing::Test
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

// Test: Jobs with no resource conflicts execute immediately
TEST_F(SchedulerHazardIntegrationTest, NoConflicts)
{
    std::atomic<int> counter{0};

    // Submit two jobs with different resources
    JobDesc desc1{
        .name = "Job1",
        .reads = {1, 2},
        .writes = {3}
    };

    JobDesc desc2{
        .name = "Job2",
        .reads = {4, 5},
        .writes = {6}
    };

    auto handle1 = Scheduler::Instance().Submit(desc1, [&counter]() {
        counter.fetch_add(1);
    });

    auto handle2 = Scheduler::Instance().Submit(desc2, [&counter]() {
        counter.fetch_add(1);
    });

    // Wait for both jobs
    Scheduler::Instance().Wait(handle1);
    Scheduler::Instance().Wait(handle2);

    // Both jobs should have executed
    EXPECT_EQ(counter.load(), 2);
}

// Test: Write-write conflict causes deferral
TEST_F(SchedulerHazardIntegrationTest, WriteWriteConflict)
{
    std::atomic<int> executionOrder{0};
    std::atomic<int> job1Order{0};
    std::atomic<int> job2Order{0};

    // Both jobs write to resource 1
    JobDesc desc1{
        .name = "Job1",
        .writes = {1}
    };

    JobDesc desc2{
        .name = "Job2",
        .writes = {1}
    };

    auto handle1 = Scheduler::Instance().Submit(desc1, [&executionOrder, &job1Order]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        job1Order.store(executionOrder.fetch_add(1));
    });

    auto handle2 = Scheduler::Instance().Submit(desc2, [&executionOrder, &job2Order]() {
        job2Order.store(executionOrder.fetch_add(1));
    });

    // Wait for both jobs
    Scheduler::Instance().Wait(handle1);
    Scheduler::Instance().Wait(handle2);

    // Both jobs should have executed
    EXPECT_EQ(executionOrder.load(), 2);

    // Job2 should have been deferred until Job1 completed
    // (execution order should be sequential, not concurrent)
    EXPECT_NE(job1Order.load(), job2Order.load());
}

// Test: Read-write conflict causes deferral
TEST_F(SchedulerHazardIntegrationTest, ReadWriteConflict)
{
    std::atomic<int> counter{0};

    // Job1 writes to resource 1
    JobDesc desc1{
        .name = "Job1_Writer",
        .writes = {1}
    };

    // Job2 reads from resource 1
    JobDesc desc2{
        .name = "Job2_Reader",
        .reads = {1}
    };

    auto handle1 = Scheduler::Instance().Submit(desc1, [&counter]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        counter.fetch_add(1);
    });

    auto handle2 = Scheduler::Instance().Submit(desc2, [&counter]() {
        counter.fetch_add(10);
    });

    // Wait for both jobs
    Scheduler::Instance().Wait(handle1);
    Scheduler::Instance().Wait(handle2);

    // Both jobs should have executed
    EXPECT_EQ(counter.load(), 11);
}

// Test: Multiple readers can execute concurrently
TEST_F(SchedulerHazardIntegrationTest, MultipleReaders)
{
    std::atomic<int> concurrentReaders{0};
    std::atomic<int> maxConcurrentReaders{0};

    auto readerJob = [&concurrentReaders, &maxConcurrentReaders]() {
        int current = concurrentReaders.fetch_add(1) + 1;
        
        // Update max concurrent readers
        int expected = maxConcurrentReaders.load();
        while (current > expected && 
               !maxConcurrentReaders.compare_exchange_weak(expected, current))
        {
            expected = maxConcurrentReaders.load();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        concurrentReaders.fetch_sub(1);
    };

    // Submit multiple reader jobs for the same resource
    JobDesc desc{
        .name = "Reader",
        .reads = {1}
    };

    std::vector<JobHandle> handles;
    for (int i = 0; i < 5; ++i)
    {
        handles.push_back(Scheduler::Instance().Submit(desc, readerJob));
    }

    // Wait for all jobs
    for (auto handle : handles)
    {
        Scheduler::Instance().Wait(handle);
    }

    // Multiple readers should have executed concurrently
    EXPECT_GT(maxConcurrentReaders.load(), 1);
}

// Test: Writer blocks readers
TEST_F(SchedulerHazardIntegrationTest, WriterBlocksReaders)
{
    std::atomic<bool> writerExecuting{false};
    std::atomic<bool> readerExecutedDuringWrite{false};

    // Writer job
    JobDesc writerDesc{
        .name = "Writer",
        .writes = {1}
    };

    // Reader job
    JobDesc readerDesc{
        .name = "Reader",
        .reads = {1}
    };

    auto writerHandle = Scheduler::Instance().Submit(writerDesc, [&writerExecuting]() {
        writerExecuting.store(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        writerExecuting.store(false);
    });

    // Give writer time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    auto readerHandle = Scheduler::Instance().Submit(readerDesc, [&writerExecuting, &readerExecutedDuringWrite]() {
        if (writerExecuting.load())
        {
            readerExecutedDuringWrite.store(true);
        }
    });

    // Wait for both jobs
    Scheduler::Instance().Wait(writerHandle);
    Scheduler::Instance().Wait(readerHandle);

    // Reader should NOT have executed while writer was running
    EXPECT_FALSE(readerExecutedDuringWrite.load());
}

// Test: Complex resource dependencies
TEST_F(SchedulerHazardIntegrationTest, ComplexDependencies)
{
    std::atomic<int> counter{0};

    // Job1: Reads {1, 2}, Writes {3}
    JobDesc desc1{
        .name = "Job1",
        .reads = {1, 2},
        .writes = {3}
    };

    // Job2: Reads {3, 4}, Writes {5} - Conflicts with Job1 (reads 3 while Job1 writes 3)
    JobDesc desc2{
        .name = "Job2",
        .reads = {3, 4},
        .writes = {5}
    };

    // Job3: Reads {5}, Writes {6} - Conflicts with Job2 (reads 5 while Job2 writes 5)
    JobDesc desc3{
        .name = "Job3",
        .reads = {5},
        .writes = {6}
    };

    auto handle1 = Scheduler::Instance().Submit(desc1, [&counter]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        counter.fetch_add(1);
    });

    auto handle2 = Scheduler::Instance().Submit(desc2, [&counter]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        counter.fetch_add(10);
    });

    auto handle3 = Scheduler::Instance().Submit(desc3, [&counter]() {
        counter.fetch_add(100);
    });

    // Wait for all jobs
    Scheduler::Instance().Wait(handle1);
    Scheduler::Instance().Wait(handle2);
    Scheduler::Instance().Wait(handle3);

    // All jobs should have executed
    EXPECT_EQ(counter.load(), 111);
}

// Test: Deferred jobs eventually execute
TEST_F(SchedulerHazardIntegrationTest, DeferredJobsExecute)
{
    std::atomic<int> executedCount{0};

    // Submit many jobs that all conflict on the same resource
    JobDesc desc{
        .name = "ConflictingJob",
        .writes = {1}
    };

    std::vector<JobHandle> handles;
    for (int i = 0; i < 10; ++i)
    {
        handles.push_back(Scheduler::Instance().Submit(desc, [&executedCount]() {
            executedCount.fetch_add(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }));
    }

    // Wait for all jobs
    for (auto handle : handles)
    {
        Scheduler::Instance().Wait(handle);
    }

    // All jobs should eventually execute
    EXPECT_EQ(executedCount.load(), 10);
}

