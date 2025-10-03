// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - ParallelFor Scaling Tests
// License: MIT

#include <gtest/gtest.h>
#include "Jobs/Scheduler.hpp"
#include "Core/Platform/Platform.hpp"
#include "Core/Platform/Time.hpp"
#include <vector>
#include <numeric>
#include <chrono>
#include <algorithm>

using namespace Engine;
using namespace Engine::Jobs;

class ParallelForTest : public ::testing::Test
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

// Test: ParallelFor correctness with simple increment
TEST_F(ParallelForTest, SimpleIncrement)
{
    constexpr usize size = 10000;
    std::vector<int> data(size, 0);

    Scheduler::Instance().ParallelFor(0, size, 100, [&data](usize i) {
        data[i] = static_cast<int>(i);
    });

    // Verify all elements were set correctly
    for (usize i = 0; i < size; ++i)
    {
        EXPECT_EQ(data[i], static_cast<int>(i));
    }
}

// Test: ParallelFor with different grain sizes
TEST_F(ParallelForTest, VariableGrainSizes)
{
    constexpr usize size = 1000;

    // Test with grain sizes: 1, 10, 100, 500, 1000
    std::vector<usize> grainSizes = {1, 10, 100, 500, 1000};

    for (usize grain : grainSizes)
    {
        std::vector<int> data(size, 0);

        Scheduler::Instance().ParallelFor(0, size, grain, [&data](usize i) {
            data[i] = static_cast<int>(i * 2);
        });

        // Verify correctness
        for (usize i = 0; i < size; ++i)
        {
            EXPECT_EQ(data[i], static_cast<int>(i * 2)) 
                << "Failed with grain size " << grain << " at index " << i;
        }
    }
}

// Test: ParallelFor with empty range
TEST_F(ParallelForTest, EmptyRange)
{
    std::vector<int> data(10, 0);

    // Empty range (begin >= end)
    Scheduler::Instance().ParallelFor(5, 5, 10, [&data](usize i) {
        data[i] = 999; // Should never execute
    });

    // Verify no elements were modified
    for (int val : data)
    {
        EXPECT_EQ(val, 0);
    }
}

// Test: ParallelFor with single element
TEST_F(ParallelForTest, SingleElement)
{
    std::vector<int> data(10, 0);

    Scheduler::Instance().ParallelFor(5, 6, 10, [&data](usize i) {
        data[i] = 42;
    });

    EXPECT_EQ(data[5], 42);
    
    // Verify other elements unchanged
    for (usize i = 0; i < 10; ++i)
    {
        if (i != 5)
        {
            EXPECT_EQ(data[i], 0);
        }
    }
}

// Test: ParallelFor with complex computation
TEST_F(ParallelForTest, ComplexComputation)
{
    constexpr usize size = 1000;
    std::vector<double> input(size);
    std::vector<double> output(size, 0.0);

    // Initialize input
    for (usize i = 0; i < size; ++i)
    {
        input[i] = static_cast<double>(i);
    }

    // Parallel computation: output[i] = sqrt(input[i]) + sin(input[i])
    Scheduler::Instance().ParallelFor(0, size, 50, [&input, &output](usize i) {
        output[i] = std::sqrt(input[i]) + std::sin(input[i]);
    });

    // Verify results
    for (usize i = 0; i < size; ++i)
    {
        double expected = std::sqrt(input[i]) + std::sin(input[i]);
        EXPECT_NEAR(output[i], expected, 1e-10);
    }
}

// Test: ParallelFor with atomic operations
TEST_F(ParallelForTest, AtomicOperations)
{
    constexpr usize size = 10000;
    std::atomic<usize> sum{0};

    Scheduler::Instance().ParallelFor(0, size, 100, [&sum](usize i) {
        sum.fetch_add(i, std::memory_order_relaxed);
    });

    // Expected sum: 0 + 1 + 2 + ... + (size-1) = size * (size-1) / 2
    usize expected = size * (size - 1) / 2;
    EXPECT_EQ(sum.load(), expected);
}

// Benchmark: ParallelFor vs sequential for large array
TEST_F(ParallelForTest, LargeArrayPerformance)
{
    constexpr usize size = 1000000;
    std::vector<double> data(size);

    // Sequential version
    auto seqStart = std::chrono::high_resolution_clock::now();
    
    for (usize i = 0; i < size; ++i)
    {
        data[i] = std::sqrt(static_cast<double>(i)) * 2.0;
    }
    
    auto seqEnd = std::chrono::high_resolution_clock::now();
    auto seqTime = std::chrono::duration_cast<std::chrono::milliseconds>(seqEnd - seqStart);

    // Reset data
    std::fill(data.begin(), data.end(), 0.0);

    // Parallel version
    auto parStart = std::chrono::high_resolution_clock::now();
    
    Scheduler::Instance().ParallelFor(0, size, 1000, [&data](usize i) {
        data[i] = std::sqrt(static_cast<double>(i)) * 2.0;
    });
    
    auto parEnd = std::chrono::high_resolution_clock::now();
    auto parTime = std::chrono::duration_cast<std::chrono::milliseconds>(parEnd - parStart);

    double speedup = static_cast<double>(seqTime.count()) / parTime.count();

    std::cout << "[Performance] Large array (1M elements):" << std::endl;
    std::cout << "  Sequential: " << seqTime.count() << " ms" << std::endl;
    std::cout << "  Parallel: " << parTime.count() << " ms" << std::endl;
    std::cout << "  Speedup: " << speedup << "x" << std::endl;

    // Expect at least some speedup
    EXPECT_GT(speedup, 1.0);
}

// Benchmark: Scaling with different array sizes
TEST_F(ParallelForTest, ScalingBenchmark)
{
    std::vector<usize> sizes = {1000, 10000, 100000, 1000000};

    std::cout << "[Performance] Scaling benchmark:" << std::endl;

    for (usize size : sizes)
    {
        std::vector<int> data(size);

        auto start = std::chrono::high_resolution_clock::now();
        
        Scheduler::Instance().ParallelFor(0, size, 1000, [&data](usize i) {
            data[i] = static_cast<int>(i * i);
        });
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        double usPerElement = static_cast<double>(duration.count()) / size;

        std::cout << "  Size " << size << ": " << duration.count() / 1000.0 
                  << " ms (" << usPerElement << " μs/element)" << std::endl;
    }
}

// Test: ParallelFor with optimal grain size
TEST_F(ParallelForTest, OptimalGrainSize)
{
    constexpr usize size = 100000;
    std::vector<int> data(size);

    // Test different grain sizes and measure performance
    std::vector<usize> grainSizes = {10, 100, 1000, 10000};
    std::vector<long long> times;

    for (usize grain : grainSizes)
    {
        std::fill(data.begin(), data.end(), 0);

        auto start = std::chrono::high_resolution_clock::now();
        
        Scheduler::Instance().ParallelFor(0, size, grain, [&data](usize i) {
            data[i] = static_cast<int>(i);
        });
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        times.push_back(duration.count());
    }

    std::cout << "[Performance] Grain size impact (100K elements):" << std::endl;
    for (usize i = 0; i < grainSizes.size(); ++i)
    {
        std::cout << "  Grain " << grainSizes[i] << ": " 
                  << times[i] / 1000.0 << " ms" << std::endl;
    }

    // All grain sizes should complete successfully
    for (usize i = 0; i < size; ++i)
    {
        EXPECT_EQ(data[i], static_cast<int>(i));
    }
}

// Test: ParallelFor with nested data structures
TEST_F(ParallelForTest, NestedStructures)
{
    constexpr usize rows = 100;
    constexpr usize cols = 100;
    std::vector<std::vector<int>> matrix(rows, std::vector<int>(cols, 0));

    // Fill matrix in parallel (row-wise)
    Scheduler::Instance().ParallelFor(0, rows, 10, [&matrix, cols](usize row) {
        for (usize col = 0; col < cols; ++col)
        {
            matrix[row][col] = static_cast<int>(row * cols + col);
        }
    });

    // Verify
    for (usize row = 0; row < rows; ++row)
    {
        for (usize col = 0; col < cols; ++col)
        {
            EXPECT_EQ(matrix[row][col], static_cast<int>(row * cols + col));
        }
    }
}

// Test: ParallelFor with lambda capture
TEST_F(ParallelForTest, LambdaCapture)
{
    constexpr usize size = 1000;
    std::vector<int> data(size, 0);
    int multiplier = 5;
    int offset = 10;

    Scheduler::Instance().ParallelFor(0, size, 100, [&data, multiplier, offset](usize i) {
        data[i] = static_cast<int>(i) * multiplier + offset;
    });

    for (usize i = 0; i < size; ++i)
    {
        EXPECT_EQ(data[i], static_cast<int>(i) * multiplier + offset);
    }
}

// Benchmark: Memory bandwidth test
TEST_F(ParallelForTest, MemoryBandwidth)
{
    constexpr usize size = 10000000; // 10M elements
    std::vector<double> src(size, 1.0);
    std::vector<double> dst(size, 0.0);

    auto start = std::chrono::high_resolution_clock::now();
    
    Scheduler::Instance().ParallelFor(0, size, 10000, [&src, &dst](usize i) {
        dst[i] = src[i] * 2.0;
    });
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Calculate bandwidth (bytes/sec)
    usize bytesProcessed = size * sizeof(double) * 2; // Read + Write
    double bandwidth = (bytesProcessed / (1024.0 * 1024.0)) / (duration.count() / 1000.0);

    std::cout << "[Performance] Memory bandwidth:" << std::endl;
    std::cout << "  Time: " << duration.count() << " ms" << std::endl;
    std::cout << "  Bandwidth: " << bandwidth << " MB/s" << std::endl;

    // Verify correctness
    for (usize i = 0; i < std::min(size, usize(100)); ++i)
    {
        EXPECT_DOUBLE_EQ(dst[i], 2.0);
    }
}

