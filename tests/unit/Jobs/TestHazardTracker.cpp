// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - Hazard Tracker Tests
// License: MIT

#include <gtest/gtest.h>
#include "Jobs/HazardTracker.hpp"
#include "Jobs/Types.hpp"
#include <vector>
#include <thread>

using namespace Engine;
using namespace Engine::Jobs;

class HazardTrackerTest : public ::testing::Test
{
protected:
    HazardTracker tracker;
};

// Test: No conflicts with empty resource sets
TEST_F(HazardTrackerTest, EmptyResourceSets)
{
    std::vector<ResourceKey> empty;
    
    EXPECT_TRUE(tracker.CanExecute(empty, empty));
    
    // Acquire and release should not crash
    tracker.AcquireResources(empty, empty);
    tracker.ReleaseResources(empty, empty);
}

// Test: Single reader can execute
TEST_F(HazardTrackerTest, SingleReader)
{
    std::vector<ResourceKey> reads = {1};
    std::vector<ResourceKey> writes;
    
    EXPECT_TRUE(tracker.CanExecute(reads, writes));
    
    tracker.AcquireResources(reads, writes);
    tracker.ReleaseResources(reads, writes);
}

// Test: Single writer can execute
TEST_F(HazardTrackerTest, SingleWriter)
{
    std::vector<ResourceKey> reads;
    std::vector<ResourceKey> writes = {1};
    
    EXPECT_TRUE(tracker.CanExecute(reads, writes));
    
    tracker.AcquireResources(reads, writes);
    tracker.ReleaseResources(reads, writes);
}

// Test: Multiple readers can execute concurrently
TEST_F(HazardTrackerTest, MultipleReaders)
{
    std::vector<ResourceKey> reads = {1};
    std::vector<ResourceKey> writes;
    
    // First reader
    EXPECT_TRUE(tracker.CanExecute(reads, writes));
    tracker.AcquireResources(reads, writes);
    
    // Second reader (should be allowed)
    EXPECT_TRUE(tracker.CanExecute(reads, writes));
    tracker.AcquireResources(reads, writes);
    
    // Third reader (should be allowed)
    EXPECT_TRUE(tracker.CanExecute(reads, writes));
    tracker.AcquireResources(reads, writes);
    
    // Release all
    tracker.ReleaseResources(reads, writes);
    tracker.ReleaseResources(reads, writes);
    tracker.ReleaseResources(reads, writes);
}

// Test: Writer blocks reader
TEST_F(HazardTrackerTest, WriterBlocksReader)
{
    std::vector<ResourceKey> reads = {1};
    std::vector<ResourceKey> writes = {1};
    
    // Acquire write lock
    EXPECT_TRUE(tracker.CanExecute({}, writes));
    tracker.AcquireResources({}, writes);
    
    // Reader should be blocked
    EXPECT_FALSE(tracker.CanExecute(reads, {}));
    
    // Release write lock
    tracker.ReleaseResources({}, writes);
    
    // Reader should now be allowed
    EXPECT_TRUE(tracker.CanExecute(reads, {}));
}

// Test: Reader blocks writer
TEST_F(HazardTrackerTest, ReaderBlocksWriter)
{
    std::vector<ResourceKey> reads = {1};
    std::vector<ResourceKey> writes = {1};
    
    // Acquire read lock
    EXPECT_TRUE(tracker.CanExecute(reads, {}));
    tracker.AcquireResources(reads, {});
    
    // Writer should be blocked
    EXPECT_FALSE(tracker.CanExecute({}, writes));
    
    // Release read lock
    tracker.ReleaseResources(reads, {});
    
    // Writer should now be allowed
    EXPECT_TRUE(tracker.CanExecute({}, writes));
}

// Test: Writer blocks writer
TEST_F(HazardTrackerTest, WriterBlocksWriter)
{
    std::vector<ResourceKey> writes = {1};
    
    // First writer
    EXPECT_TRUE(tracker.CanExecute({}, writes));
    tracker.AcquireResources({}, writes);
    
    // Second writer should be blocked
    EXPECT_FALSE(tracker.CanExecute({}, writes));
    
    // Release first writer
    tracker.ReleaseResources({}, writes);
    
    // Second writer should now be allowed
    EXPECT_TRUE(tracker.CanExecute({}, writes));
}

// Test: Multiple resources - no conflicts
TEST_F(HazardTrackerTest, MultipleResourcesNoConflict)
{
    std::vector<ResourceKey> reads1 = {1, 2};
    std::vector<ResourceKey> writes1 = {3};

    // Fixed: Use completely different resources to avoid conflicts
    std::vector<ResourceKey> reads2 = {4, 5};
    std::vector<ResourceKey> writes2 = {6};

    // First job
    EXPECT_TRUE(tracker.CanExecute(reads1, writes1));
    tracker.AcquireResources(reads1, writes1);

    // Second job (completely different resources, should be allowed)
    EXPECT_TRUE(tracker.CanExecute(reads2, writes2));
    tracker.AcquireResources(reads2, writes2);

    // Release
    tracker.ReleaseResources(reads1, writes1);
    tracker.ReleaseResources(reads2, writes2);
}

// Test: Multiple resources - with conflicts
TEST_F(HazardTrackerTest, MultipleResourcesWithConflict)
{
    std::vector<ResourceKey> reads1 = {1, 2, 3};
    std::vector<ResourceKey> writes1 = {4};
    
    std::vector<ResourceKey> reads2 = {5};
    std::vector<ResourceKey> writes2 = {2, 6}; // Conflicts with reads1
    
    // First job
    EXPECT_TRUE(tracker.CanExecute(reads1, writes1));
    tracker.AcquireResources(reads1, writes1);
    
    // Second job (conflicts on resource 2)
    EXPECT_FALSE(tracker.CanExecute(reads2, writes2));
    
    // Release first job
    tracker.ReleaseResources(reads1, writes1);
    
    // Second job should now be allowed
    EXPECT_TRUE(tracker.CanExecute(reads2, writes2));
}

// Test: Read and write same resource
TEST_F(HazardTrackerTest, ReadAndWriteSameResource)
{
    std::vector<ResourceKey> reads = {1};
    std::vector<ResourceKey> writes = {1};
    
    // Job that both reads and writes resource 1
    EXPECT_TRUE(tracker.CanExecute(reads, writes));
    tracker.AcquireResources(reads, writes);
    
    // Another job trying to read resource 1 (should be blocked)
    EXPECT_FALSE(tracker.CanExecute(reads, {}));
    
    // Another job trying to write resource 1 (should be blocked)
    EXPECT_FALSE(tracker.CanExecute({}, writes));
    
    // Release
    tracker.ReleaseResources(reads, writes);
    
    // Now both should be allowed
    EXPECT_TRUE(tracker.CanExecute(reads, {}));
    EXPECT_TRUE(tracker.CanExecute({}, writes));
}

// Test: Resource cleanup after release
TEST_F(HazardTrackerTest, ResourceCleanup)
{
    std::vector<ResourceKey> reads = {1};
    std::vector<ResourceKey> writes = {2};
    
    // Acquire
    tracker.AcquireResources(reads, writes);
    
    // Release
    tracker.ReleaseResources(reads, writes);
    
    // Resources should be cleaned up and available
    EXPECT_TRUE(tracker.CanExecute(reads, {}));
    EXPECT_TRUE(tracker.CanExecute({}, writes));
    EXPECT_TRUE(tracker.CanExecute(reads, writes));
}

// Test: Stress test with many resources
TEST_F(HazardTrackerTest, ManyResources)
{
    constexpr usize numResources = 1000;
    std::vector<ResourceKey> reads;
    std::vector<ResourceKey> writes;
    
    // Create large resource sets
    for (usize i = 0; i < numResources; ++i)
    {
        if (i % 2 == 0)
            reads.push_back(i);
        else
            writes.push_back(i);
    }
    
    // Should be able to acquire
    EXPECT_TRUE(tracker.CanExecute(reads, writes));
    tracker.AcquireResources(reads, writes);
    
    // Conflicting access should be blocked
    EXPECT_FALSE(tracker.CanExecute({}, {0})); // 0 is in reads
    EXPECT_FALSE(tracker.CanExecute({1}, {})); // 1 is in writes
    
    // Release
    tracker.ReleaseResources(reads, writes);
    
    // Should be available again
    EXPECT_TRUE(tracker.CanExecute({}, {0}));
    EXPECT_TRUE(tracker.CanExecute({1}, {}));
}

// Test: Concurrent access from multiple threads
TEST_F(HazardTrackerTest, ConcurrentAccess)
{
    constexpr usize numThreads = 4;
    constexpr usize opsPerThread = 100;
    
    std::vector<std::thread> threads;
    
    for (usize t = 0; t < numThreads; ++t)
    {
        threads.emplace_back([this, t]() {
            for (usize i = 0; i < opsPerThread; ++i)
            {
                ResourceKey resource = t * 100 + i;
                std::vector<ResourceKey> reads = {resource};
                std::vector<ResourceKey> writes;
                
                // Acquire
                if (tracker.CanExecute(reads, writes))
                {
                    tracker.AcquireResources(reads, writes);
                    
                    // Simulate some work
                    std::this_thread::yield();
                    
                    // Release
                    tracker.ReleaseResources(reads, writes);
                }
            }
        });
    }
    
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    // All resources should be released
    for (usize t = 0; t < numThreads; ++t)
    {
        for (usize i = 0; i < opsPerThread; ++i)
        {
            ResourceKey resource = t * 100 + i;
            EXPECT_TRUE(tracker.CanExecute({resource}, {}));
        }
    }
}

// Test: Overlapping resource sets
TEST_F(HazardTrackerTest, OverlappingResourceSets)
{
    std::vector<ResourceKey> reads1 = {1, 2, 3};
    std::vector<ResourceKey> writes1 = {4, 5};
    
    std::vector<ResourceKey> reads2 = {3, 4, 5};
    std::vector<ResourceKey> writes2 = {6};
    
    // First job
    tracker.AcquireResources(reads1, writes1);
    
    // Second job conflicts on:
    // - Resource 3 (read vs read - OK)
    // - Resource 4 (read vs write - CONFLICT)
    // - Resource 5 (read vs write - CONFLICT)
    EXPECT_FALSE(tracker.CanExecute(reads2, writes2));
    
    // Release first job
    tracker.ReleaseResources(reads1, writes1);
    
    // Second job should now be allowed
    EXPECT_TRUE(tracker.CanExecute(reads2, writes2));
}

// Test: Partial release (edge case)
TEST_F(HazardTrackerTest, PartialRelease)
{
    std::vector<ResourceKey> reads = {1, 2};
    std::vector<ResourceKey> writes = {3, 4};
    
    // Acquire all
    tracker.AcquireResources(reads, writes);
    
    // Release only reads
    tracker.ReleaseResources(reads, {});
    
    // Reads should be available
    EXPECT_TRUE(tracker.CanExecute(reads, {}));
    
    // Writes should still be locked
    EXPECT_FALSE(tracker.CanExecute({}, writes));
    
    // Release writes
    tracker.ReleaseResources({}, writes);
    
    // All should be available
    EXPECT_TRUE(tracker.CanExecute(reads, writes));
}

