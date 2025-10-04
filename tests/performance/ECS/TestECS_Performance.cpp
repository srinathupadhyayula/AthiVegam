#include <gtest/gtest.h>
#include "ECS/World.hpp"
#include "ECS/ParallelQuery.hpp"
#include "Jobs/Scheduler.hpp"
#include "Core/Logger.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <numeric>
#include <algorithm>

using namespace Engine::ECS;
using namespace Engine::Jobs;

// Test components
struct Position {
    float x, y, z;
};

struct Velocity {
    float dx, dy, dz;
};

struct Transform {
    float x, y, z;
    float rx, ry, rz;
};

struct RigidBody {
    float mass;
    float friction;
};

// Performance measurement helper
struct PerformanceResult {
    std::string testName;
    size_t operationCount;
    double totalTimeMs;
    double avgTimeUs;
    double opsPerSecond;
    
    void Print() const {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "\n=== " << testName << " ===\n";
        std::cout << "Operations: " << operationCount << "\n";
        std::cout << "Total Time: " << totalTimeMs << " ms\n";
        std::cout << "Avg Time: " << avgTimeUs << " μs/op\n";
        std::cout << "Throughput: " << opsPerSecond << " ops/sec\n";
    }
};

class PerformanceBenchmark {
public:
    static PerformanceResult Measure(const std::string& name, size_t operations, std::function<void()> func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double totalMs = duration.count() / 1000.0;
        double avgUs = static_cast<double>(duration.count()) / operations;
        double opsPerSec = (operations * 1000000.0) / duration.count();
        
        return PerformanceResult{name, operations, totalMs, avgUs, opsPerSec};
    }
};

// Test fixture
class ECS_Performance : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Initialize Logger
        Engine::LoggerConfig logConfig;
        logConfig.consoleLevel = Engine::LogLevel::Warn;
        logConfig.enableFile = false;
        logConfig.enableConsole = true;
        Engine::Logger::Initialize(logConfig);

        // Initialize Jobs system
        Scheduler::Instance().Initialize();
    }

    void TearDown() override
    {
        Scheduler::Instance().Shutdown();
        Engine::Logger::Shutdown();
    }
};

// ===== Entity Creation/Destruction Benchmarks =====

TEST_F(ECS_Performance, Benchmark_EntityCreation_Sequential)
{
    World world;
    const size_t entityCount = 100000;
    
    auto result = PerformanceBenchmark::Measure(
        "Entity Creation (Sequential)",
        entityCount,
        [&]() {
            for (size_t i = 0; i < entityCount; ++i) {
                world.CreateEntity();
            }
        }
    );
    
    result.Print();
    
    // Baseline: Should create at least 100K entities/sec
    EXPECT_GT(result.opsPerSecond, 100000.0);
}

TEST_F(ECS_Performance, Benchmark_EntityCreation_WithFreeListReuse)
{
    World world;
    const size_t entityCount = 50000;
    
    // Pre-populate free list
    std::vector<Entity> entities;
    for (size_t i = 0; i < entityCount; ++i) {
        entities.push_back(world.CreateEntity());
    }
    for (const auto& e : entities) {
        world.DestroyEntity(e);
    }
    
    auto result = PerformanceBenchmark::Measure(
        "Entity Creation (Free List Reuse)",
        entityCount,
        [&]() {
            for (size_t i = 0; i < entityCount; ++i) {
                world.CreateEntity();
            }
        }
    );
    
    result.Print();
    
    // Should be fast with free list reuse
    EXPECT_GT(result.opsPerSecond, 100000.0);
}

TEST_F(ECS_Performance, Benchmark_EntityDestruction)
{
    World world;
    const size_t entityCount = 100000;
    
    // Create entities first
    std::vector<Entity> entities;
    entities.reserve(entityCount);
    for (size_t i = 0; i < entityCount; ++i) {
        entities.push_back(world.CreateEntity());
    }
    
    auto result = PerformanceBenchmark::Measure(
        "Entity Destruction",
        entityCount,
        [&]() {
            for (const auto& e : entities) {
                world.DestroyEntity(e);
            }
        }
    );
    
    result.Print();
    
    // Should destroy at least 100K entities/sec
    EXPECT_GT(result.opsPerSecond, 100000.0);
}

// ===== Component Operation Benchmarks =====

TEST_F(ECS_Performance, Benchmark_ComponentAdd)
{
    World world;
    const size_t entityCount = 50000;
    
    // Create entities
    std::vector<Entity> entities;
    entities.reserve(entityCount);
    for (size_t i = 0; i < entityCount; ++i) {
        entities.push_back(world.CreateEntity());
    }
    
    Position pos{0.0f, 0.0f, 0.0f};
    
    auto result = PerformanceBenchmark::Measure(
        "Component Add",
        entityCount,
        [&]() {
            for (const auto& e : entities) {
                world.Add(e, pos);
            }
        }
    );
    
    result.Print();
    
    // Should add at least 50K components/sec
    EXPECT_GT(result.opsPerSecond, 50000.0);
}

TEST_F(ECS_Performance, Benchmark_ComponentGet)
{
    World world;
    const size_t entityCount = 100000;
    
    // Create entities with Position
    std::vector<Entity> entities;
    entities.reserve(entityCount);
    for (size_t i = 0; i < entityCount; ++i) {
        Entity e = world.CreateEntity();
        entities.push_back(e);
        Position pos{0.0f, 0.0f, 0.0f};
        world.Add(e, pos);
    }
    
    auto result = PerformanceBenchmark::Measure(
        "Component Get",
        entityCount,
        [&]() {
            for (const auto& e : entities) {
                auto posResult = world.Get<Position>(e);
                if (posResult.has_value()) {
                    (*posResult)->x += 1.0f;
                }
            }
        }
    );
    
    result.Print();
    
    // Should get at least 200K components/sec
    EXPECT_GT(result.opsPerSecond, 200000.0);
}

TEST_F(ECS_Performance, Benchmark_ComponentRemove)
{
    World world;
    const size_t entityCount = 50000;
    
    // Create entities with Position
    std::vector<Entity> entities;
    entities.reserve(entityCount);
    for (size_t i = 0; i < entityCount; ++i) {
        Entity e = world.CreateEntity();
        entities.push_back(e);
        Position pos{0.0f, 0.0f, 0.0f};
        world.Add(e, pos);
    }
    
    auto result = PerformanceBenchmark::Measure(
        "Component Remove",
        entityCount,
        [&]() {
            for (const auto& e : entities) {
                world.Remove<Position>(e);
            }
        }
    );
    
    result.Print();
    
    // Should remove at least 50K components/sec
    EXPECT_GT(result.opsPerSecond, 50000.0);
}

TEST_F(ECS_Performance, Benchmark_ComponentHas)
{
    World world;
    const size_t entityCount = 100000;
    
    // Create entities with Position
    std::vector<Entity> entities;
    entities.reserve(entityCount);
    for (size_t i = 0; i < entityCount; ++i) {
        Entity e = world.CreateEntity();
        entities.push_back(e);
        Position pos{0.0f, 0.0f, 0.0f};
        world.Add(e, pos);
    }
    
    auto result = PerformanceBenchmark::Measure(
        "Component Has",
        entityCount,
        [&]() {
            for (const auto& e : entities) {
                volatile bool has = world.Has<Position>(e);
                (void)has;
            }
        }
    );
    
    result.Print();
    
    // Should check at least 500K components/sec
    EXPECT_GT(result.opsPerSecond, 500000.0);
}

// ===== Query Iteration Benchmarks =====

TEST_F(ECS_Performance, Benchmark_QueryIteration_SingleComponent)
{
    World world;
    const size_t entityCount = 100000;
    
    // Create entities with Position
    for (size_t i = 0; i < entityCount; ++i) {
        Entity e = world.CreateEntity();
        Position pos{static_cast<float>(i), 0.0f, 0.0f};
        world.Add(e, pos);
    }
    
    auto query = world.QueryComponents<Position>();
    
    auto result = PerformanceBenchmark::Measure(
        "Query Iteration (Single Component)",
        entityCount,
        [&]() {
            query.ForEach([](Position& pos) {
                pos.x += 1.0f;
            });
        }
    );
    
    result.Print();
    
    // Should iterate at least 1M entities/sec
    EXPECT_GT(result.opsPerSecond, 1000000.0);
}

TEST_F(ECS_Performance, Benchmark_QueryIteration_MultipleComponents)
{
    World world;
    const size_t entityCount = 100000;
    
    // Create entities with Position and Velocity
    for (size_t i = 0; i < entityCount; ++i) {
        Entity e = world.CreateEntity();
        Position pos{static_cast<float>(i), 0.0f, 0.0f};
        Velocity vel{1.0f, 0.0f, 0.0f};
        world.Add(e, pos);
        world.Add(e, vel);
    }
    
    auto query = world.QueryComponents<Position, Velocity>();
    
    auto result = PerformanceBenchmark::Measure(
        "Query Iteration (Multiple Components)",
        entityCount,
        [&]() {
            query.ForEach([](Position& pos, Velocity& vel) {
                pos.x += vel.dx;
                pos.y += vel.dy;
                pos.z += vel.dz;
            });
        }
    );
    
    result.Print();
    
    // Should iterate at least 800K entities/sec
    EXPECT_GT(result.opsPerSecond, 800000.0);
}

// ===== Parallel Execution Benchmarks =====

TEST_F(ECS_Performance, Benchmark_ParallelExecution_SingleComponent)
{
    World world;
    const size_t entityCount = 100000;

    // Create entities with Position
    for (size_t i = 0; i < entityCount; ++i) {
        Entity e = world.CreateEntity();
        Position pos{static_cast<float>(i), 0.0f, 0.0f};
        world.Add(e, pos);
    }

    auto query = world.QueryComponents<Position>();
    auto parallel = MakeParallel(query);

    auto result = PerformanceBenchmark::Measure(
        "Parallel Execution (Single Component)",
        entityCount,
        [&]() {
            parallel.Execute([](Position& pos) {
                pos.x += 1.0f;
            });
        }
    );

    result.Print();

    // Parallel should be faster than sequential
    // Should process at least 2M entities/sec with parallelism
    EXPECT_GT(result.opsPerSecond, 2000000.0);
}

TEST_F(ECS_Performance, Benchmark_ParallelExecution_MultipleComponents)
{
    World world;
    const size_t entityCount = 100000;

    // Create entities with Position and Velocity
    for (size_t i = 0; i < entityCount; ++i) {
        Entity e = world.CreateEntity();
        Position pos{static_cast<float>(i), 0.0f, 0.0f};
        Velocity vel{1.0f, 0.0f, 0.0f};
        world.Add(e, pos);
        world.Add(e, vel);
    }

    auto query = world.QueryComponents<Position, Velocity>();
    auto parallel = MakeParallel(query);

    auto result = PerformanceBenchmark::Measure(
        "Parallel Execution (Multiple Components)",
        entityCount,
        [&]() {
            parallel.Execute([](Position& pos, Velocity& vel) {
                pos.x += vel.dx;
                pos.y += vel.dy;
                pos.z += vel.dz;
            });
        }
    );

    result.Print();

    // Should process at least 1.5M entities/sec
    EXPECT_GT(result.opsPerSecond, 1500000.0);
}

TEST_F(ECS_Performance, Benchmark_ParallelExecution_ChunkLevel)
{
    World world;
    const size_t entityCount = 100000;

    // Create entities
    for (size_t i = 0; i < entityCount; ++i) {
        Entity e = world.CreateEntity();
        Position pos{static_cast<float>(i), 0.0f, 0.0f};
        Velocity vel{1.0f, 0.0f, 0.0f};
        world.Add(e, pos);
        world.Add(e, vel);
    }

    auto query = world.QueryComponents<Position, Velocity>();
    auto parallel = MakeParallel(query);

    auto result = PerformanceBenchmark::Measure(
        "Parallel Execution (Chunk Level)",
        entityCount,
        [&]() {
            parallel.ExecuteChunks([](size_t chunkIndex, Position* posColumn, Velocity* velColumn, size_t count) {
                for (size_t i = 0; i < count; ++i) {
                    posColumn[i].x += velColumn[i].dx;
                    posColumn[i].y += velColumn[i].dy;
                    posColumn[i].z += velColumn[i].dz;
                }
            });
        }
    );

    result.Print();

    // Chunk-level should be very fast due to cache locality
    // Should process at least 3M entities/sec
    EXPECT_GT(result.opsPerSecond, 3000000.0);
}

// ===== Archetype Migration Benchmarks =====

TEST_F(ECS_Performance, Benchmark_ArchetypeMigration_Add)
{
    World world;
    const size_t entityCount = 10000;

    // Create entities with Position
    std::vector<Entity> entities;
    entities.reserve(entityCount);
    for (size_t i = 0; i < entityCount; ++i) {
        Entity e = world.CreateEntity();
        entities.push_back(e);
        Position pos{0.0f, 0.0f, 0.0f};
        world.Add(e, pos);
    }

    Velocity vel{1.0f, 0.0f, 0.0f};

    auto result = PerformanceBenchmark::Measure(
        "Archetype Migration (Add Component)",
        entityCount,
        [&]() {
            for (const auto& e : entities) {
                world.Add(e, vel);
            }
        }
    );

    result.Print();

    // Migration is expensive but should still be reasonable
    // Should migrate at least 10K entities/sec
    EXPECT_GT(result.opsPerSecond, 10000.0);
}

TEST_F(ECS_Performance, Benchmark_ArchetypeMigration_Remove)
{
    World world;
    const size_t entityCount = 10000;

    // Create entities with Position and Velocity
    std::vector<Entity> entities;
    entities.reserve(entityCount);
    for (size_t i = 0; i < entityCount; ++i) {
        Entity e = world.CreateEntity();
        entities.push_back(e);
        Position pos{0.0f, 0.0f, 0.0f};
        Velocity vel{1.0f, 0.0f, 0.0f};
        world.Add(e, pos);
        world.Add(e, vel);
    }

    auto result = PerformanceBenchmark::Measure(
        "Archetype Migration (Remove Component)",
        entityCount,
        [&]() {
            for (const auto& e : entities) {
                world.Remove<Velocity>(e);
            }
        }
    );

    result.Print();

    // Should migrate at least 10K entities/sec
    EXPECT_GT(result.opsPerSecond, 10000.0);
}

// ===== Memory Usage Benchmarks =====

TEST_F(ECS_Performance, Benchmark_MemoryUsage_EntityOverhead)
{
    World world;
    const size_t entityCount = 100000;

    std::cout << "\n=== Memory Usage: Entity Overhead ===\n";
    std::cout << "Creating " << entityCount << " entities...\n";

    for (size_t i = 0; i < entityCount; ++i) {
        world.CreateEntity();
    }

    // Theoretical overhead: 12 bytes per entity (index, version, alive flag)
    size_t theoreticalBytes = entityCount * 12;
    std::cout << "Theoretical overhead: " << theoreticalBytes / 1024 << " KB\n";
    std::cout << "Bytes per entity: ~12 bytes\n";

    EXPECT_EQ(world.AliveCount(), entityCount);
}

TEST_F(ECS_Performance, Benchmark_MemoryUsage_ComponentStorage)
{
    World world;
    const size_t entityCount = 100000;

    std::cout << "\n=== Memory Usage: Component Storage ===\n";
    std::cout << "Creating " << entityCount << " entities with Position (12 bytes)...\n";

    for (size_t i = 0; i < entityCount; ++i) {
        Entity e = world.CreateEntity();
        Position pos{0.0f, 0.0f, 0.0f};
        world.Add(e, pos);
    }

    // Component storage: 12 bytes per Position
    size_t componentBytes = entityCount * sizeof(Position);
    std::cout << "Component storage: " << componentBytes / 1024 << " KB\n";
    std::cout << "Bytes per component: " << sizeof(Position) << " bytes\n";

    EXPECT_EQ(world.AliveCount(), entityCount);
}

// ===== Comparative Benchmarks =====

TEST_F(ECS_Performance, Benchmark_Comparison_SequentialVsParallel)
{
    World world;
    const size_t entityCount = 100000;

    // Create entities
    for (size_t i = 0; i < entityCount; ++i) {
        Entity e = world.CreateEntity();
        Position pos{static_cast<float>(i), 0.0f, 0.0f};
        Velocity vel{1.0f, 0.0f, 0.0f};
        world.Add(e, pos);
        world.Add(e, vel);
    }

    auto query = world.QueryComponents<Position, Velocity>();
    auto parallel = MakeParallel(query);

    // Sequential
    auto seqResult = PerformanceBenchmark::Measure(
        "Sequential Iteration",
        entityCount,
        [&]() {
            query.ForEach([](Position& pos, Velocity& vel) {
                pos.x += vel.dx;
                pos.y += vel.dy;
                pos.z += vel.dz;
            });
        }
    );

    // Parallel
    auto parResult = PerformanceBenchmark::Measure(
        "Parallel Iteration",
        entityCount,
        [&]() {
            parallel.Execute([](Position& pos, Velocity& vel) {
                pos.x += vel.dx;
                pos.y += vel.dy;
                pos.z += vel.dz;
            });
        }
    );

    std::cout << "\n=== Sequential vs Parallel Comparison ===\n";
    seqResult.Print();
    parResult.Print();

    double speedup = parResult.opsPerSecond / seqResult.opsPerSecond;
    std::cout << "\nSpeedup: " << std::fixed << std::setprecision(2) << speedup << "x\n";

    // Parallel should be faster
    EXPECT_GT(parResult.opsPerSecond, seqResult.opsPerSecond);
}


