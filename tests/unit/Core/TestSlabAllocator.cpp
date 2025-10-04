// AthiVegam Engine - SlabAllocator Unit Tests
// License: MIT

#include <gtest/gtest.h>
#include "Core/Memory/SlabAllocator.hpp"
#include "Core/Types.hpp"
#include <thread>
#include <vector>
#include <atomic>

using namespace Engine::Memory;
using namespace Engine;

// Test object with tracking
struct TestObject {
    int value = 0;
    static inline std::atomic<int> constructCount{0};
    static inline std::atomic<int> destructCount{0};
    
    TestObject() { constructCount.fetch_add(1, std::memory_order_relaxed); }
    TestObject(int v) : value(v) { constructCount.fetch_add(1, std::memory_order_relaxed); }
    ~TestObject() { destructCount.fetch_add(1, std::memory_order_relaxed); }
    
    static void ResetCounts() {
        constructCount = 0;
        destructCount = 0;
    }
};

// ============================================================================
// Construction Tests
// ============================================================================

TEST(SlabAllocator, Construction_Default)
{
    SlabAllocator<int> slab;
    
    EXPECT_EQ(slab.AllocatedCount(), 0);
    EXPECT_GE(slab.Capacity(), 0);
}

TEST(SlabAllocator, Construction_WithCapacity)
{
    SlabAllocator<int> slab(100);
    
    EXPECT_EQ(slab.AllocatedCount(), 0);
}

// ============================================================================
// Basic Allocation Tests
// ============================================================================

TEST(SlabAllocator, Allocate_Single)
{
    SlabAllocator<int> slab;
    
    Handle<int> handle = slab.Allocate();
    
    EXPECT_TRUE(handle.IsValid());
    EXPECT_EQ(slab.AllocatedCount(), 1);
}

TEST(SlabAllocator, Allocate_Multiple)
{
    SlabAllocator<int> slab;
    
    std::vector<Handle<int>> handles;
    for (int i = 0; i < 10; ++i)
    {
        handles.push_back(slab.Allocate());
    }
    
    EXPECT_EQ(slab.AllocatedCount(), 10);
    
    // All handles should be valid and unique
    for (size_t i = 0; i < handles.size(); ++i)
    {
        EXPECT_TRUE(handles[i].IsValid());
        for (size_t j = i + 1; j < handles.size(); ++j)
        {
            EXPECT_NE(handles[i].Index(), handles[j].Index());
        }
    }
}

TEST(SlabAllocator, Get_ValidHandle)
{
    SlabAllocator<int> slab;
    
    Handle<int> handle = slab.Allocate();
    int* ptr = slab.Get(handle);
    
    ASSERT_NE(ptr, nullptr);
    *ptr = 42;
    EXPECT_EQ(*ptr, 42);
}

TEST(SlabAllocator, Get_InvalidHandle)
{
    SlabAllocator<int> slab;
    
    Handle<int> invalidHandle;
    int* ptr = slab.Get(invalidHandle);
    
    EXPECT_EQ(ptr, nullptr);
}

TEST(SlabAllocator, Get_Const)
{
    SlabAllocator<int> slab;
    
    Handle<int> handle = slab.Allocate();
    int* ptr = slab.Get(handle);
    *ptr = 123;
    
    const SlabAllocator<int>& constSlab = slab;
    const int* constPtr = constSlab.Get(handle);
    
    ASSERT_NE(constPtr, nullptr);
    EXPECT_EQ(*constPtr, 123);
}

// ============================================================================
// Deallocation Tests
// ============================================================================

TEST(SlabAllocator, Deallocate_Single)
{
    SlabAllocator<int> slab;
    
    Handle<int> handle = slab.Allocate();
    EXPECT_EQ(slab.AllocatedCount(), 1);
    
    slab.Deallocate(handle);
    
    EXPECT_EQ(slab.AllocatedCount(), 0);
}

TEST(SlabAllocator, Deallocate_InvalidHandle)
{
    SlabAllocator<int> slab;
    
    Handle<int> invalidHandle;
    
    // Should not crash
    slab.Deallocate(invalidHandle);
    
    EXPECT_EQ(slab.AllocatedCount(), 0);
}

TEST(SlabAllocator, Deallocate_Multiple)
{
    SlabAllocator<int> slab;
    
    std::vector<Handle<int>> handles;
    for (int i = 0; i < 10; ++i)
    {
        handles.push_back(slab.Allocate());
    }
    
    for (auto handle : handles)
    {
        slab.Deallocate(handle);
    }
    
    EXPECT_EQ(slab.AllocatedCount(), 0);
}

// ============================================================================
// Handle Versioning Tests
// ============================================================================

TEST(SlabAllocator, HandleVersioning_AfterDeallocate)
{
    SlabAllocator<int> slab;
    
    Handle<int> handle1 = slab.Allocate();
    u32 index1 = handle1.Index();
    u32 version1 = handle1.Version();
    
    slab.Deallocate(handle1);
    
    // Old handle should now be invalid
    EXPECT_FALSE(slab.IsValid(handle1));
    EXPECT_EQ(slab.Get(handle1), nullptr);
    
    // Allocate again (should reuse same index)
    Handle<int> handle2 = slab.Allocate();
    u32 index2 = handle2.Index();
    u32 version2 = handle2.Version();
    
    EXPECT_EQ(index1, index2); // Same index
    EXPECT_NE(version1, version2); // Different version
    EXPECT_EQ(version2, version1 + 1); // Version incremented
}

TEST(SlabAllocator, HandleVersioning_OldHandleInvalid)
{
    SlabAllocator<int> slab;
    
    Handle<int> oldHandle = slab.Allocate();
    int* ptr1 = slab.Get(oldHandle);
    *ptr1 = 100;
    
    slab.Deallocate(oldHandle);
    
    Handle<int> newHandle = slab.Allocate();
    int* ptr2 = slab.Get(newHandle);
    *ptr2 = 200;
    
    // Old handle should not access new object
    EXPECT_FALSE(slab.IsValid(oldHandle));
    EXPECT_EQ(slab.Get(oldHandle), nullptr);
    
    // New handle should work
    EXPECT_TRUE(slab.IsValid(newHandle));
    EXPECT_EQ(*slab.Get(newHandle), 200);
}

TEST(SlabAllocator, HandleVersioning_MultipleReuse)
{
    SlabAllocator<int> slab;
    
    Handle<int> handle = slab.Allocate();
    u32 initialVersion = handle.Version();
    
    // Deallocate and reallocate multiple times
    for (int i = 0; i < 5; ++i)
    {
        slab.Deallocate(handle);
        handle = slab.Allocate();
        EXPECT_EQ(handle.Version(), initialVersion + i + 1);
    }
}

// ============================================================================
// IsValid Tests
// ============================================================================

TEST(SlabAllocator, IsValid_ValidHandle)
{
    SlabAllocator<int> slab;
    
    Handle<int> handle = slab.Allocate();
    
    EXPECT_TRUE(slab.IsValid(handle));
}

TEST(SlabAllocator, IsValid_InvalidHandle)
{
    SlabAllocator<int> slab;
    
    Handle<int> invalidHandle;
    
    EXPECT_FALSE(slab.IsValid(invalidHandle));
}

TEST(SlabAllocator, IsValid_DeallocatedHandle)
{
    SlabAllocator<int> slab;
    
    Handle<int> handle = slab.Allocate();
    slab.Deallocate(handle);
    
    EXPECT_FALSE(slab.IsValid(handle));
}

TEST(SlabAllocator, IsValid_OutOfBoundsIndex)
{
    SlabAllocator<int> slab;
    
    // Create handle with out-of-bounds index
    Handle<int> handle(9999, 1);
    
    EXPECT_FALSE(slab.IsValid(handle));
}

// ============================================================================
// Reuse Tests
// ============================================================================

TEST(SlabAllocator, Reuse_AfterDeallocate)
{
    SlabAllocator<int> slab;
    
    Handle<int> handle1 = slab.Allocate();
    u32 index1 = handle1.Index();
    
    slab.Deallocate(handle1);
    
    Handle<int> handle2 = slab.Allocate();
    u32 index2 = handle2.Index();
    
    // Should reuse the same slot
    EXPECT_EQ(index1, index2);
}

TEST(SlabAllocator, Reuse_LIFO_Order)
{
    SlabAllocator<int> slab;
    
    Handle<int> h1 = slab.Allocate();
    Handle<int> h2 = slab.Allocate();
    Handle<int> h3 = slab.Allocate();
    
    slab.Deallocate(h1);
    slab.Deallocate(h2);
    slab.Deallocate(h3);
    
    // Free list is LIFO
    Handle<int> n1 = slab.Allocate();
    EXPECT_EQ(n1.Index(), h3.Index());
    
    Handle<int> n2 = slab.Allocate();
    EXPECT_EQ(n2.Index(), h2.Index());
    
    Handle<int> n3 = slab.Allocate();
    EXPECT_EQ(n3.Index(), h1.Index());
}

// ============================================================================
// Clear Tests
// ============================================================================

TEST(SlabAllocator, Clear_Basic)
{
    SlabAllocator<int> slab;
    
    for (int i = 0; i < 10; ++i)
    {
        slab.Allocate();
    }
    
    EXPECT_EQ(slab.AllocatedCount(), 10);
    
    slab.Clear();
    
    EXPECT_EQ(slab.AllocatedCount(), 0);
    EXPECT_EQ(slab.Capacity(), 0);
}

TEST(SlabAllocator, Clear_HandlesInvalidated)
{
    SlabAllocator<int> slab;
    
    Handle<int> handle = slab.Allocate();
    EXPECT_TRUE(slab.IsValid(handle));
    
    slab.Clear();
    
    EXPECT_FALSE(slab.IsValid(handle));
}

// ============================================================================
// Object Lifecycle Tests
// ============================================================================

TEST(SlabAllocator, ObjectLifecycle_ConstructorCalled)
{
    TestObject::ResetCounts();
    
    {
        SlabAllocator<TestObject> slab;
        slab.Allocate();
        
        EXPECT_EQ(TestObject::constructCount.load(), 1);
    }
}

TEST(SlabAllocator, ObjectLifecycle_DestructorCalled)
{
    TestObject::ResetCounts();
    
    {
        SlabAllocator<TestObject> slab;
        Handle<TestObject> handle = slab.Allocate();
        slab.Deallocate(handle);
        
        EXPECT_EQ(TestObject::destructCount.load(), 1);
    }
}

TEST(SlabAllocator, ObjectLifecycle_MultipleObjects)
{
    TestObject::ResetCounts();
    
    {
        SlabAllocator<TestObject> slab;
        
        for (int i = 0; i < 10; ++i)
        {
            Handle<TestObject> handle = slab.Allocate();
            TestObject* obj = slab.Get(handle);
            obj->value = i;
        }
        
        EXPECT_EQ(TestObject::constructCount.load(), 10);
    }
}

// ============================================================================
// Move Semantics Tests
// ============================================================================

TEST(SlabAllocator, MoveConstruction)
{
    SlabAllocator<int> slab1;
    Handle<int> handle = slab1.Allocate();
    int* ptr = slab1.Get(handle);
    *ptr = 42;
    
    SlabAllocator<int> slab2(std::move(slab1));
    
    EXPECT_EQ(slab2.AllocatedCount(), 1);
    EXPECT_EQ(*slab2.Get(handle), 42);
}

TEST(SlabAllocator, MoveAssignment)
{
    SlabAllocator<int> slab1;
    Handle<int> handle = slab1.Allocate();
    int* ptr = slab1.Get(handle);
    *ptr = 123;
    
    SlabAllocator<int> slab2;
    slab2 = std::move(slab1);
    
    EXPECT_EQ(slab2.AllocatedCount(), 1);
    EXPECT_EQ(*slab2.Get(handle), 123);
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST(SlabAllocator, Stress_ManyAllocations)
{
    SlabAllocator<int> slab;
    
    std::vector<Handle<int>> handles;
    for (int i = 0; i < 1000; ++i)
    {
        handles.push_back(slab.Allocate());
    }
    
    EXPECT_EQ(slab.AllocatedCount(), 1000);
    
    // Verify all handles are valid
    for (auto handle : handles)
    {
        EXPECT_TRUE(slab.IsValid(handle));
    }
}

TEST(SlabAllocator, Stress_AllocateDeallocateCycle)
{
    SlabAllocator<int> slab;
    
    for (int cycle = 0; cycle < 100; ++cycle)
    {
        std::vector<Handle<int>> handles;
        
        // Allocate
        for (int i = 0; i < 50; ++i)
        {
            handles.push_back(slab.Allocate());
        }
        
        EXPECT_EQ(slab.AllocatedCount(), 50);
        
        // Deallocate
        for (auto handle : handles)
        {
            slab.Deallocate(handle);
        }
        
        EXPECT_EQ(slab.AllocatedCount(), 0);
    }
}

TEST(SlabAllocator, Stress_RandomAllocDealloc)
{
    SlabAllocator<int> slab;
    
    std::vector<Handle<int>> allocated;
    
    for (int i = 0; i < 1000; ++i)
    {
        if (allocated.empty() || (i % 3 != 0))
        {
            // Allocate
            allocated.push_back(slab.Allocate());
        }
        else
        {
            // Deallocate random
            size_t idx = i % allocated.size();
            slab.Deallocate(allocated[idx]);
            allocated.erase(allocated.begin() + idx);
        }
    }
    
    // Cleanup
    for (auto handle : allocated)
    {
        slab.Deallocate(handle);
    }
    
    EXPECT_EQ(slab.AllocatedCount(), 0);
}

// ============================================================================
// Complex Object Tests
// ============================================================================

struct ComplexObject {
    std::vector<int> data;
    std::string name;
    
    ComplexObject() : data{1, 2, 3}, name("default") {}
};

TEST(SlabAllocator, ComplexObject_Allocation)
{
    SlabAllocator<ComplexObject> slab;
    
    Handle<ComplexObject> handle = slab.Allocate();
    ComplexObject* obj = slab.Get(handle);
    
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->data.size(), 3);
    EXPECT_EQ(obj->name, "default");
    
    obj->data.push_back(4);
    obj->name = "modified";
    
    EXPECT_EQ(obj->data.size(), 4);
    EXPECT_EQ(obj->name, "modified");
}

TEST(SlabAllocator, ComplexObject_Deallocation)
{
    SlabAllocator<ComplexObject> slab;
    
    Handle<ComplexObject> handle = slab.Allocate();
    ComplexObject* obj = slab.Get(handle);
    obj->data.resize(1000);
    
    // Should properly destruct and free memory
    slab.Deallocate(handle);
    
    EXPECT_EQ(slab.AllocatedCount(), 0);
}
