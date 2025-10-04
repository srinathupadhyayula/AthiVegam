// AthiVegam Engine - PoolAllocator Unit Tests
// License: MIT

#include <gtest/gtest.h>
#include "Core/Memory/PoolAllocator.hpp"
#include "Core/Types.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <unordered_set>

using namespace Engine::Memory;

// ============================================================================
// Construction Tests
// ============================================================================

TEST(PoolAllocator, Construction_Valid)
{
    PoolAllocator pool(64, 8, 100);
    
    EXPECT_EQ(pool.BlockSize(), 64);
    EXPECT_EQ(pool.BlockCount(), 100);
    EXPECT_EQ(pool.AllocatedBlocks(), 0);
    EXPECT_EQ(pool.FreeBlocks(), 100);
    EXPECT_TRUE(pool.IsEmpty());
    EXPECT_FALSE(pool.IsFull());
}

TEST(PoolAllocator, Construction_ZeroBlockSize)
{
    PoolAllocator pool(0, 8, 100);
    
    // Should handle gracefully
    EXPECT_EQ(pool.BlockCount(), 0);
}

TEST(PoolAllocator, Construction_ZeroBlockCount)
{
    PoolAllocator pool(64, 8, 0);
    
    // Should handle gracefully
    EXPECT_EQ(pool.BlockCount(), 0);
}

TEST(PoolAllocator, Construction_SmallBlockSize)
{
    // Block size smaller than pointer size should be adjusted
    PoolAllocator pool(1, 8, 10);
    
    EXPECT_GE(pool.BlockSize(), sizeof(void*));
}

TEST(PoolAllocator, Construction_InvalidAlignment)
{
    // Non-power-of-2 alignment
    PoolAllocator pool(64, 7, 100);
    
    // Should handle gracefully (likely fails to allocate)
    EXPECT_EQ(pool.BlockCount(), 0);
}

// ============================================================================
// Basic Allocation Tests
// ============================================================================

TEST(PoolAllocator, Allocate_Single)
{
    PoolAllocator pool(64, 8, 10);
    
    void* ptr = pool.Allocate(64, 8);
    
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(pool.AllocatedBlocks(), 1);
    EXPECT_EQ(pool.FreeBlocks(), 9);
    EXPECT_FALSE(pool.IsEmpty());
    EXPECT_FALSE(pool.IsFull());
}

TEST(PoolAllocator, Allocate_Multiple)
{
    PoolAllocator pool(64, 8, 10);
    
    std::vector<void*> ptrs;
    for (int i = 0; i < 5; ++i)
    {
        void* ptr = pool.Allocate(64, 8);
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }
    
    EXPECT_EQ(pool.AllocatedBlocks(), 5);
    EXPECT_EQ(pool.FreeBlocks(), 5);
    
    // All pointers should be unique
    std::unordered_set<void*> uniquePtrs(ptrs.begin(), ptrs.end());
    EXPECT_EQ(uniquePtrs.size(), ptrs.size());
}

TEST(PoolAllocator, Allocate_UntilFull)
{
    PoolAllocator pool(64, 8, 10);
    
    std::vector<void*> ptrs;
    for (int i = 0; i < 10; ++i)
    {
        void* ptr = pool.Allocate(64, 8);
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }
    
    EXPECT_TRUE(pool.IsFull());
    EXPECT_EQ(pool.FreeBlocks(), 0);
    
    // Next allocation should fail
    void* ptr = pool.Allocate(64, 8);
    EXPECT_EQ(ptr, nullptr);
}

TEST(PoolAllocator, Allocate_SizeMismatch)
{
    PoolAllocator pool(64, 8, 10);
    
    // Request size larger than block size
    void* ptr = pool.Allocate(128, 8);
    
    EXPECT_EQ(ptr, nullptr);
}

TEST(PoolAllocator, Allocate_AlignmentMismatch)
{
    PoolAllocator pool(64, 8, 10);
    
    // Request alignment larger than block alignment
    void* ptr = pool.Allocate(64, 16);
    
    EXPECT_EQ(ptr, nullptr);
}

// ============================================================================
// Deallocation Tests
// ============================================================================

TEST(PoolAllocator, Deallocate_Single)
{
    PoolAllocator pool(64, 8, 10);
    
    void* ptr = pool.Allocate(64, 8);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(pool.AllocatedBlocks(), 1);
    
    pool.Deallocate(ptr);
    
    EXPECT_EQ(pool.AllocatedBlocks(), 0);
    EXPECT_EQ(pool.FreeBlocks(), 10);
    EXPECT_TRUE(pool.IsEmpty());
}

TEST(PoolAllocator, Deallocate_Multiple)
{
    PoolAllocator pool(64, 8, 10);
    
    std::vector<void*> ptrs;
    for (int i = 0; i < 5; ++i)
    {
        ptrs.push_back(pool.Allocate(64, 8));
    }
    
    for (void* ptr : ptrs)
    {
        pool.Deallocate(ptr);
    }
    
    EXPECT_TRUE(pool.IsEmpty());
    EXPECT_EQ(pool.FreeBlocks(), 10);
}

TEST(PoolAllocator, Deallocate_Nullptr)
{
    PoolAllocator pool(64, 8, 10);
    
    // Should not crash
    pool.Deallocate(nullptr);
    
    EXPECT_TRUE(pool.IsEmpty());
}

TEST(PoolAllocator, Deallocate_InvalidPointer)
{
    PoolAllocator pool(64, 8, 10);
    
    int dummy = 0;
    void* invalidPtr = &dummy;
    
    // Should handle gracefully (logs error but doesn't crash)
    pool.Deallocate(invalidPtr);
    
    EXPECT_TRUE(pool.IsEmpty());
}

TEST(PoolAllocator, Deallocate_DoubleFree)
{
    PoolAllocator pool(64, 8, 10);
    
    void* ptr = pool.Allocate(64, 8);
    ASSERT_NE(ptr, nullptr);
    
    pool.Deallocate(ptr);
    EXPECT_EQ(pool.AllocatedBlocks(), 0);
    
    // Double free should be detected and handled
    pool.Deallocate(ptr);
    
    // Should still be empty (not negative count)
    EXPECT_EQ(pool.AllocatedBlocks(), 0);
}

// ============================================================================
// Reuse Tests
// ============================================================================

TEST(PoolAllocator, Reuse_AfterDeallocate)
{
    PoolAllocator pool(64, 8, 10);
    
    void* ptr1 = pool.Allocate(64, 8);
    ASSERT_NE(ptr1, nullptr);
    
    pool.Deallocate(ptr1);
    
    void* ptr2 = pool.Allocate(64, 8);
    ASSERT_NE(ptr2, nullptr);
    
    // Should reuse the same block
    EXPECT_EQ(ptr1, ptr2);
}

TEST(PoolAllocator, Reuse_LIFO_Order)
{
    PoolAllocator pool(64, 8, 10);
    
    void* ptr1 = pool.Allocate(64, 8);
    void* ptr2 = pool.Allocate(64, 8);
    void* ptr3 = pool.Allocate(64, 8);
    
    pool.Deallocate(ptr1);
    pool.Deallocate(ptr2);
    pool.Deallocate(ptr3);
    
    // Free list is LIFO, so should get ptr3 first
    void* newPtr1 = pool.Allocate(64, 8);
    EXPECT_EQ(newPtr1, ptr3);
    
    void* newPtr2 = pool.Allocate(64, 8);
    EXPECT_EQ(newPtr2, ptr2);
    
    void* newPtr3 = pool.Allocate(64, 8);
    EXPECT_EQ(newPtr3, ptr1);
}

// ============================================================================
// Alignment Tests
// ============================================================================

TEST(PoolAllocator, Alignment_Correct)
{
    PoolAllocator pool(64, 16, 10);
    
    for (int i = 0; i < 10; ++i)
    {
        void* ptr = pool.Allocate(64, 16);
        ASSERT_NE(ptr, nullptr);
        EXPECT_TRUE(IsAligned(ptr, 16));
    }
}

TEST(PoolAllocator, Alignment_Large)
{
    PoolAllocator pool(128, 64, 10);
    
    void* ptr = pool.Allocate(128, 64);
    
    ASSERT_NE(ptr, nullptr);
    EXPECT_TRUE(IsAligned(ptr, 64));
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(PoolAllocator, EdgeCase_SingleBlock)
{
    PoolAllocator pool(64, 8, 1);
    
    void* ptr1 = pool.Allocate(64, 8);
    ASSERT_NE(ptr1, nullptr);
    EXPECT_TRUE(pool.IsFull());
    
    void* ptr2 = pool.Allocate(64, 8);
    EXPECT_EQ(ptr2, nullptr);
    
    pool.Deallocate(ptr1);
    EXPECT_TRUE(pool.IsEmpty());
    
    void* ptr3 = pool.Allocate(64, 8);
    ASSERT_NE(ptr3, nullptr);
    EXPECT_EQ(ptr3, ptr1);
}

TEST(PoolAllocator, EdgeCase_LargeBlockSize)
{
    PoolAllocator pool(4096, 64, 10);
    
    void* ptr = pool.Allocate(4096, 64);
    
    ASSERT_NE(ptr, nullptr);
    EXPECT_TRUE(IsAligned(ptr, 64));
}

// ============================================================================
// Interface Tests
// ============================================================================

TEST(PoolAllocator, IAllocator_Interface)
{
    PoolAllocator pool(64, 8, 10);
    IAllocator* allocator = &pool;
    
    void* ptr = allocator->Allocate(64, 8);
    ASSERT_NE(ptr, nullptr);
    
    EXPECT_EQ(allocator->Allocated(), 64);
    EXPECT_STREQ(allocator->Name(), "PoolAllocator");
    
    allocator->Deallocate(ptr);
    EXPECT_EQ(allocator->Allocated(), 0);
}

// ============================================================================
// Move Semantics Tests
// ============================================================================

TEST(PoolAllocator, MoveConstruction)
{
    PoolAllocator pool1(64, 8, 10);
    void* ptr = pool1.Allocate(64, 8);
    ASSERT_NE(ptr, nullptr);
    
    PoolAllocator pool2(std::move(pool1));
    
    EXPECT_EQ(pool2.BlockSize(), 64);
    EXPECT_EQ(pool2.BlockCount(), 10);
    EXPECT_EQ(pool2.AllocatedBlocks(), 1);
}

TEST(PoolAllocator, MoveAssignment)
{
    PoolAllocator pool1(64, 8, 10);
    void* ptr = pool1.Allocate(64, 8);
    ASSERT_NE(ptr, nullptr);
    
    PoolAllocator pool2(32, 4, 5);
    pool2 = std::move(pool1);
    
    EXPECT_EQ(pool2.BlockSize(), 64);
    EXPECT_EQ(pool2.BlockCount(), 10);
    EXPECT_EQ(pool2.AllocatedBlocks(), 1);
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST(PoolAllocator, Stress_AllocateDeallocateCycle)
{
    PoolAllocator pool(64, 8, 100);
    
    for (int cycle = 0; cycle < 1000; ++cycle)
    {
        std::vector<void*> ptrs;
        
        // Allocate all blocks
        for (int i = 0; i < 100; ++i)
        {
            void* ptr = pool.Allocate(64, 8);
            ASSERT_NE(ptr, nullptr);
            ptrs.push_back(ptr);
        }
        
        EXPECT_TRUE(pool.IsFull());
        
        // Deallocate all blocks
        for (void* ptr : ptrs)
        {
            pool.Deallocate(ptr);
        }
        
        EXPECT_TRUE(pool.IsEmpty());
    }
}

TEST(PoolAllocator, Stress_RandomAllocDealloc)
{
    PoolAllocator pool(64, 8, 50);
    
    std::vector<void*> allocated;
    
    for (int i = 0; i < 1000; ++i)
    {
        if (allocated.empty() || (allocated.size() < 50 && (i % 3 != 0)))
        {
            // Allocate
            void* ptr = pool.Allocate(64, 8);
            if (ptr != nullptr)
            {
                allocated.push_back(ptr);
            }
        }
        else
        {
            // Deallocate random block
            size_t idx = i % allocated.size();
            pool.Deallocate(allocated[idx]);
            allocated.erase(allocated.begin() + idx);
        }
    }
    
    // Cleanup
    for (void* ptr : allocated)
    {
        pool.Deallocate(ptr);
    }
    
    EXPECT_TRUE(pool.IsEmpty());
}

// ============================================================================
// Thread Safety Tests (PoolAllocator is NOT thread-safe by design)
// ============================================================================

TEST(PoolAllocator, Concurrent_Allocations)
{
    // This test documents that PoolAllocator is NOT thread-safe
    // In production, use external synchronization or per-thread pools
    
    PoolAllocator pool(64, 8, 1000);
    
    std::atomic<int> successCount{0};
    constexpr int numThreads = 4;
    constexpr int allocsPerThread = 100;
    
    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; ++t)
    {
        threads.emplace_back([&]() {
            std::vector<void*> localPtrs;
            for (int i = 0; i < allocsPerThread; ++i)
            {
                void* ptr = pool.Allocate(64, 8);
                if (ptr != nullptr)
                {
                    localPtrs.push_back(ptr);
                    successCount.fetch_add(1, std::memory_order_relaxed);
                }
            }
            
            // Deallocate
            for (void* ptr : localPtrs)
            {
                pool.Deallocate(ptr);
            }
        });
    }
    
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    // Due to data races, results may be unpredictable
    // This test just ensures no crashes
    EXPECT_GT(successCount.load(), 0);
}
