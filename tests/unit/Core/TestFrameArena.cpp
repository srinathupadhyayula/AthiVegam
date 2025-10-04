// AthiVegam Engine - FrameArena Unit Tests
// License: MIT

#include <gtest/gtest.h>
#include "Core/Memory/FrameArena.hpp"
#include "Core/Types.hpp"
#include <thread>
#include <vector>
#include <atomic>

using namespace Engine::Memory;
using namespace Engine;

// ============================================================================
// Basic Allocation Tests
// ============================================================================

TEST(FrameArena, Construction_ValidCapacity)
{
    FrameArena arena(1024);
    
    EXPECT_EQ(arena.Capacity(), 1024);
    EXPECT_EQ(arena.Used(), 0);
    EXPECT_EQ(arena.Remaining(), 1024);
    EXPECT_EQ(arena.Allocated(), 0);
}

TEST(FrameArena, Construction_ZeroCapacity)
{
    FrameArena arena(0);
    
    // Should handle zero capacity gracefully
    EXPECT_EQ(arena.Capacity(), 0);
    EXPECT_EQ(arena.Used(), 0);
}

TEST(FrameArena, Allocate_Basic)
{
    FrameArena arena(1024);
    
    void* ptr = arena.Allocate(64);
    
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(arena.Used(), 64);
    EXPECT_EQ(arena.Remaining(), 1024 - 64);
    EXPECT_EQ(arena.Allocated(), 64);
}

TEST(FrameArena, Allocate_ZeroSize)
{
    FrameArena arena(1024);
    
    void* ptr = arena.Allocate(0);
    
    EXPECT_EQ(ptr, nullptr);
    EXPECT_EQ(arena.Used(), 0);
}

TEST(FrameArena, Allocate_ExceedsCapacity)
{
    FrameArena arena(100);
    
    void* ptr = arena.Allocate(200);
    
    EXPECT_EQ(ptr, nullptr);
    EXPECT_EQ(arena.Used(), 0);
}

// ============================================================================
// Alignment Tests
// ============================================================================

TEST(FrameArena, Allocate_DefaultAlignment)
{
    FrameArena arena(1024);
    
    void* ptr = arena.Allocate(1);
    
    ASSERT_NE(ptr, nullptr);
    EXPECT_TRUE(IsAligned(ptr, alignof(std::max_align_t)));
}

TEST(FrameArena, Allocate_CustomAlignment)
{
    FrameArena arena(1024);
    
    void* ptr1 = arena.Allocate(1, 16);
    ASSERT_NE(ptr1, nullptr);
    EXPECT_TRUE(IsAligned(ptr1, 16));
    
    void* ptr2 = arena.Allocate(1, 32);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_TRUE(IsAligned(ptr2, 32));
    
    void* ptr3 = arena.Allocate(1, 64);
    ASSERT_NE(ptr3, nullptr);
    EXPECT_TRUE(IsAligned(ptr3, 64));
}

TEST(FrameArena, Allocate_InvalidAlignment)
{
    FrameArena arena(1024);
    
    // Non-power-of-2 alignment should fail
    void* ptr = arena.Allocate(64, 7);
    
    EXPECT_EQ(ptr, nullptr);
}

TEST(FrameArena, Allocate_AlignmentPadding)
{
    FrameArena arena(1024);
    
    // Allocate 1 byte (will be aligned to default)
    void* ptr1 = arena.Allocate(1);
    ASSERT_NE(ptr1, nullptr);
    
    // Allocate with 64-byte alignment (may require padding)
    void* ptr2 = arena.Allocate(1, 64);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_TRUE(IsAligned(ptr2, 64));
    
    // Used space should account for padding
    EXPECT_GT(arena.Used(), 2);
}

// ============================================================================
// Multiple Allocation Tests
// ============================================================================

TEST(FrameArena, Allocate_Multiple)
{
    FrameArena arena(1024);
    
    void* ptr1 = arena.Allocate(64);
    void* ptr2 = arena.Allocate(128);
    void* ptr3 = arena.Allocate(256);
    
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);
    
    // Pointers should be different
    EXPECT_NE(ptr1, ptr2);
    EXPECT_NE(ptr2, ptr3);
    EXPECT_NE(ptr1, ptr3);
    
    // Total used should be at least sum of allocations
    EXPECT_GE(arena.Used(), 64 + 128 + 256);
}

TEST(FrameArena, Allocate_UntilFull)
{
    FrameArena arena(256);
    
    std::vector<void*> allocations;
    
    // Allocate until full
    while (true)
    {
        void* ptr = arena.Allocate(32);
        if (ptr == nullptr)
            break;
        allocations.push_back(ptr);
    }
    
    // Should have allocated some blocks
    EXPECT_GT(allocations.size(), 0);
    
    // Arena should be nearly full
    EXPECT_LE(arena.Remaining(), 32);
}

// ============================================================================
// Reset Tests
// ============================================================================

TEST(FrameArena, Reset_Basic)
{
    FrameArena arena(1024);
    
    void* ptr1 = arena.Allocate(256);
    ASSERT_NE(ptr1, nullptr);
    EXPECT_EQ(arena.Used(), 256);
    
    arena.Reset();
    
    EXPECT_EQ(arena.Used(), 0);
    EXPECT_EQ(arena.Remaining(), 1024);
}

TEST(FrameArena, Reset_ReuseMemory)
{
    FrameArena arena(1024);
    
    void* ptr1 = arena.Allocate(256);
    ASSERT_NE(ptr1, nullptr);
    
    arena.Reset();
    
    void* ptr2 = arena.Allocate(256);
    ASSERT_NE(ptr2, nullptr);
    
    // After reset, should reuse the same memory
    EXPECT_EQ(ptr1, ptr2);
}

TEST(FrameArena, Reset_Multiple)
{
    FrameArena arena(1024);
    
    for (int i = 0; i < 10; ++i)
    {
        void* ptr = arena.Allocate(512);
        ASSERT_NE(ptr, nullptr);
        
        arena.Reset();
        
        EXPECT_EQ(arena.Used(), 0);
    }
}

TEST(FrameArena, Reset_EmptyArena)
{
    FrameArena arena(1024);
    
    // Reset without any allocations
    arena.Reset();
    
    EXPECT_EQ(arena.Used(), 0);
    EXPECT_EQ(arena.Remaining(), 1024);
}

// ============================================================================
// Deallocate Tests (No-op)
// ============================================================================

TEST(FrameArena, Deallocate_NoOp)
{
    FrameArena arena(1024);
    
    void* ptr = arena.Allocate(256);
    ASSERT_NE(ptr, nullptr);
    
    usize usedBefore = arena.Used();
    
    // Deallocate should be a no-op
    arena.Deallocate(ptr);
    
    EXPECT_EQ(arena.Used(), usedBefore);
}

TEST(FrameArena, Deallocate_Nullptr)
{
    FrameArena arena(1024);
    
    // Should not crash
    arena.Deallocate(nullptr);
    
    EXPECT_EQ(arena.Used(), 0);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(FrameArena, Allocate_ExactCapacity)
{
    FrameArena arena(256);
    
    void* ptr = arena.Allocate(256);
    
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(arena.Used(), 256);
    EXPECT_EQ(arena.Remaining(), 0);
}

TEST(FrameArena, Allocate_NearCapacity)
{
    FrameArena arena(256);
    
    void* ptr1 = arena.Allocate(200);
    ASSERT_NE(ptr1, nullptr);
    
    void* ptr2 = arena.Allocate(50);
    ASSERT_NE(ptr2, nullptr);
    
    // Next allocation should fail
    void* ptr3 = arena.Allocate(10);
    EXPECT_EQ(ptr3, nullptr);
}

TEST(FrameArena, Allocate_LargeAlignment)
{
    FrameArena arena(1024);
    
    // Allocate with very large alignment
    void* ptr = arena.Allocate(64, 256);
    
    ASSERT_NE(ptr, nullptr);
    EXPECT_TRUE(IsAligned(ptr, 256));
}

// ============================================================================
// Interface Tests
// ============================================================================

TEST(FrameArena, IAllocator_Interface)
{
    FrameArena arena(1024);
    IAllocator* allocator = &arena;
    
    void* ptr = allocator->Allocate(128);
    ASSERT_NE(ptr, nullptr);
    
    EXPECT_EQ(allocator->Allocated(), 128);
    EXPECT_STREQ(allocator->Name(), "FrameArena");
    
    allocator->Deallocate(ptr);
}

// ============================================================================
// Move Semantics Tests
// ============================================================================

TEST(FrameArena, MoveConstruction)
{
    FrameArena arena1(1024);
    void* ptr1 = arena1.Allocate(256);
    ASSERT_NE(ptr1, nullptr);
    
    FrameArena arena2(std::move(arena1));
    
    EXPECT_EQ(arena2.Capacity(), 1024);
    EXPECT_EQ(arena2.Used(), 256);
}

TEST(FrameArena, MoveAssignment)
{
    FrameArena arena1(1024);
    void* ptr1 = arena1.Allocate(256);
    ASSERT_NE(ptr1, nullptr);
    
    FrameArena arena2(512);
    arena2 = std::move(arena1);
    
    EXPECT_EQ(arena2.Capacity(), 1024);
    EXPECT_EQ(arena2.Used(), 256);
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST(FrameArena, Stress_ManySmallAllocations)
{
    FrameArena arena(64 * 1024); // 64 KB
    
    std::vector<void*> allocations;
    
    // Allocate many small blocks
    for (int i = 0; i < 1000; ++i)
    {
        void* ptr = arena.Allocate(32);
        if (ptr != nullptr)
        {
            allocations.push_back(ptr);
        }
    }
    
    EXPECT_GT(allocations.size(), 0);
}

TEST(FrameArena, Stress_ResetCycle)
{
    FrameArena arena(1024);
    
    // Simulate frame-based allocation pattern
    for (int frame = 0; frame < 1000; ++frame)
    {
        // Allocate some memory
        for (int i = 0; i < 10; ++i)
        {
            void* ptr = arena.Allocate(64);
            (void)ptr;
        }
        
        // Reset at end of frame
        arena.Reset();
        
        EXPECT_EQ(arena.Used(), 0);
    }
}

// ============================================================================
// Thread Safety Tests (FrameArena is NOT thread-safe by design)
// ============================================================================

TEST(FrameArena, Concurrent_Allocations_DataRace)
{
    // This test documents that FrameArena is NOT thread-safe
    // In production, each thread should have its own FrameArena
    
    FrameArena arena(64 * 1024);
    
    std::atomic<int> successCount{0};
    constexpr int numThreads = 4;
    constexpr int allocsPerThread = 100;
    
    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; ++t)
    {
        threads.emplace_back([&]() {
            for (int i = 0; i < allocsPerThread; ++i)
            {
                void* ptr = arena.Allocate(64);
                if (ptr != nullptr)
                {
                    successCount.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }
    
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    // Due to data races, success count may be less than expected
    // This test just ensures no crashes occur
    EXPECT_GT(successCount.load(), 0);
}
