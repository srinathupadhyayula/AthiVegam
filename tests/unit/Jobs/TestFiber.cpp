// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - Fiber Tests
// License: MIT

#include <gtest/gtest.h>
#include "Jobs/Fiber.hpp"
#include "Core/Platform/Platform.hpp"
#include <atomic>

using namespace Engine;
using namespace Engine::Jobs;

class FiberTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Platform::Initialize();
    }

    void TearDown() override
    {
        Platform::Shutdown();
    }
};

#ifdef _WIN32

// Test: Create and delete a fiber
TEST_F(FiberTest, CreateAndDelete)
{
    std::atomic<bool> executed{false};

    FiberHandle fiber = Fiber::Create(0, [&executed]() {
        executed.store(true);
    });

    ASSERT_NE(fiber, nullptr);

    // Delete the fiber (should not leak memory)
    Fiber::Delete(fiber);

    // Note: We don't execute the fiber in this test, just verify creation/deletion
}

// Test: Create multiple fibers and delete them
TEST_F(FiberTest, CreateMultipleFibers)
{
    constexpr usize numFibers = 10;
    std::vector<FiberHandle> fibers;

    // Create fibers
    for (usize i = 0; i < numFibers; ++i)
    {
        FiberHandle fiber = Fiber::Create(0, []() {
            // Empty function
        });

        ASSERT_NE(fiber, nullptr);
        fibers.push_back(fiber);
    }

    // Delete all fibers (should not leak memory)
    for (auto fiber : fibers)
    {
        Fiber::Delete(fiber);
    }
}

// Test: Thread to fiber conversion
TEST_F(FiberTest, ConvertThreadToFiber)
{
    FiberHandle threadFiber = Fiber::ConvertThreadToFiber();
    ASSERT_NE(threadFiber, nullptr);

    // Convert back to thread
    Fiber::ConvertFiberToThread();
}

// Test: Get current fiber
TEST_F(FiberTest, GetCurrentFiber)
{
    // Convert thread to fiber first
    FiberHandle threadFiber = Fiber::ConvertThreadToFiber();
    ASSERT_NE(threadFiber, nullptr);

    // Get current fiber
    FiberHandle current = Fiber::GetCurrent();
    EXPECT_EQ(current, threadFiber);

    // Convert back to thread
    Fiber::ConvertFiberToThread();
}

// Test: Fiber with custom stack size
TEST_F(FiberTest, CustomStackSize)
{
    constexpr usize stackSize = 1024 * 1024; // 1MB

    FiberHandle fiber = Fiber::Create(stackSize, []() {
        // Empty function
    });

    ASSERT_NE(fiber, nullptr);
    Fiber::Delete(fiber);
}

// Test: Fiber with captured variables
TEST_F(FiberTest, CapturedVariables)
{
    int value = 42;
    std::atomic<int> result{0};

    FiberHandle fiber = Fiber::Create(0, [value, &result]() {
        result.store(value);
    });

    ASSERT_NE(fiber, nullptr);

    // Note: We don't execute the fiber, just verify it can be created with captures
    Fiber::Delete(fiber);
}

// Test: Create fiber with null function (should handle gracefully)
TEST_F(FiberTest, NullFunction)
{
    FiberHandle fiber = Fiber::Create(0, nullptr);

    // Should still create the fiber (function check happens at execution time)
    ASSERT_NE(fiber, nullptr);
    Fiber::Delete(fiber);
}

// Test: Delete null fiber (should not crash)
TEST_F(FiberTest, DeleteNullFiber)
{
    Fiber::Delete(nullptr);
    // Should not crash
}

// Test: Memory leak verification (create and delete many fibers)
TEST_F(FiberTest, NoMemoryLeaks)
{
    constexpr usize iterations = 1000;

    for (usize i = 0; i < iterations; ++i)
    {
        FiberHandle fiber = Fiber::Create(0, [i]() {
            // Capture index
        });

        ASSERT_NE(fiber, nullptr);
        Fiber::Delete(fiber);
    }

    // If there are memory leaks, this test will show them in memory profilers
    // The fix in Fiber.cpp should prevent FiberContext leaks
}

#else

// Stub tests for non-Windows platforms
TEST_F(FiberTest, NotImplemented)
{
    FiberHandle fiber = Fiber::Create(0, []() {});
    EXPECT_EQ(fiber, nullptr); // Should return nullptr on unsupported platforms
}

#endif

