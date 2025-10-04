#include <gtest/gtest.h>
#include "ECS/World.hpp"
#include "ECS/ParallelQuery.hpp"
#include "Jobs/Scheduler.hpp"
#include "Core/Logger.hpp"
#include <chrono>
#include <thread>
#include <atomic>
#include <random>

using namespace Engine::ECS;
using namespace Engine::Jobs;

// Test components
struct Position {
    float x, y, z;
};

struct Velocity {
    float dx, dy, dz;
};

struct Health {
    int current;
    int max;
};

struct Tag {
    uint32_t value;
};

// Test fixture with Jobs system initialization
class ECS_Integration : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Initialize Logger
        Engine::LoggerConfig logConfig;
        logConfig.consoleLevel = Engine::LogLevel::Info;
        logConfig.enableFile = false;
        logConfig.enableConsole = true;
        Engine::Logger::Initialize(logConfig);

        // Initialize Jobs system
        Scheduler::Instance().Initialize();
    }

    void TearDown() override
    {
        // Shutdown Jobs system
        Scheduler::Instance().Shutdown();

        // Shutdown Logger
        Engine::Logger::Shutdown();
    }
};

// ===== Large-Scale Entity Creation Tests =====

TEST_F(ECS_Integration, LargeScale_10K_Entities_SingleComponent)
{
    World world;
    const size_t entityCount = 10000;

    auto start = std::chrono::high_resolution_clock::now();

    // Create 10K entities with Position
    std::vector<Entity> entities;
    entities.reserve(entityCount);

    for (size_t i = 0; i < entityCount; ++i)
    {
        Entity e = world.CreateEntity();
        entities.push_back(e);
        
        Position pos{ static_cast<float>(i), 0.0f, 0.0f };
        ASSERT_TRUE(world.Add(e, pos).has_value());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Verify all entities created
    EXPECT_EQ(world.AliveCount(), entityCount);

    // Verify all entities have Position
    for (const auto& e : entities)
    {
        EXPECT_TRUE(world.Has<Position>(e));
    }

    // Performance check: should complete in reasonable time (< 1 second)
    EXPECT_LT(duration.count(), 1000) << "10K entity creation took " << duration.count() << "ms";
}

TEST_F(ECS_Integration, LargeScale_10K_Entities_MultipleComponents)
{
    World world;
    const size_t entityCount = 10000;

    auto start = std::chrono::high_resolution_clock::now();

    // Create 10K entities with Position, Velocity, and Health
    std::vector<Entity> entities;
    entities.reserve(entityCount);

    for (size_t i = 0; i < entityCount; ++i)
    {
        Entity e = world.CreateEntity();
        entities.push_back(e);
        
        Position pos{ static_cast<float>(i), 0.0f, 0.0f };
        Velocity vel{ 1.0f, 0.0f, 0.0f };
        Health health{ 100, 100 };
        
        ASSERT_TRUE(world.Add(e, pos).has_value());
        ASSERT_TRUE(world.Add(e, vel).has_value());
        ASSERT_TRUE(world.Add(e, health).has_value());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Verify all entities created
    EXPECT_EQ(world.AliveCount(), entityCount);

    // Verify all entities have all components
    for (const auto& e : entities)
    {
        EXPECT_TRUE(world.Has<Position>(e));
        EXPECT_TRUE(world.Has<Velocity>(e));
        EXPECT_TRUE(world.Has<Health>(e));
    }

    // Performance check: should complete in reasonable time (< 2 seconds)
    EXPECT_LT(duration.count(), 2000) << "10K entity creation with 3 components took " << duration.count() << "ms";
}

TEST_F(ECS_Integration, LargeScale_ParallelQuery_10K_Entities)
{
    World world;
    const size_t entityCount = 10000;

    // Create entities
    for (size_t i = 0; i < entityCount; ++i)
    {
        Entity e = world.CreateEntity();
        Position pos{ static_cast<float>(i), 0.0f, 0.0f };
        Velocity vel{ 1.0f, 0.0f, 0.0f };
        world.Add(e, pos);
        world.Add(e, vel);
    }

    auto query = world.QueryComponents<Position, Velocity>();
    auto parallel = MakeParallel(query);

    auto start = std::chrono::high_resolution_clock::now();

    // Execute parallel update
    parallel.Execute([](Position& pos, Velocity& vel) {
        pos.x += vel.dx;
        pos.y += vel.dy;
        pos.z += vel.dz;
    });

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Verify all positions updated
    query.ForEach([](Position& pos, Velocity& vel) {
        EXPECT_GE(pos.x, 1.0f);
    });

    // Performance check: parallel execution should be fast (< 100ms)
    EXPECT_LT(duration.count(), 100) << "Parallel query on 10K entities took " << duration.count() << "ms";
}

// ===== Stress Tests =====

TEST_F(ECS_Integration, Stress_RapidCreateDestroy_1000_Cycles)
{
    World world;
    const size_t cycles = 1000;
    const size_t entitiesPerCycle = 100;

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t cycle = 0; cycle < cycles; ++cycle)
    {
        // Create entities
        std::vector<Entity> entities;
        entities.reserve(entitiesPerCycle);

        for (size_t i = 0; i < entitiesPerCycle; ++i)
        {
            Entity e = world.CreateEntity();
            entities.push_back(e);
            
            Position pos{ 0.0f, 0.0f, 0.0f };
            world.Add(e, pos);
        }

        EXPECT_EQ(world.AliveCount(), entitiesPerCycle);

        // Destroy all entities
        for (const auto& e : entities)
        {
            ASSERT_TRUE(world.DestroyEntity(e).has_value());
        }

        EXPECT_EQ(world.AliveCount(), 0u);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Performance check: should complete in reasonable time (< 5 seconds)
    EXPECT_LT(duration.count(), 5000) << "1000 create/destroy cycles took " << duration.count() << "ms";
}

TEST_F(ECS_Integration, Stress_ArchetypeMigration_1000_Entities)
{
    World world;
    const size_t entityCount = 1000;

    // Create entities with Position
    std::vector<Entity> entities;
    entities.reserve(entityCount);

    for (size_t i = 0; i < entityCount; ++i)
    {
        Entity e = world.CreateEntity();
        entities.push_back(e);
        
        Position pos{ 0.0f, 0.0f, 0.0f };
        world.Add(e, pos);
    }

    auto start = std::chrono::high_resolution_clock::now();

    // Add Velocity to all entities (archetype migration)
    for (const auto& e : entities)
    {
        Velocity vel{ 1.0f, 0.0f, 0.0f };
        ASSERT_TRUE(world.Add(e, vel).has_value());
    }

    // Add Health to all entities (another migration)
    for (const auto& e : entities)
    {
        Health health{ 100, 100 };
        ASSERT_TRUE(world.Add(e, health).has_value());
    }

    // Remove Velocity from all entities (migration back)
    for (const auto& e : entities)
    {
        ASSERT_TRUE(world.Remove<Velocity>(e).has_value());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Verify final state
    for (const auto& e : entities)
    {
        EXPECT_TRUE(world.Has<Position>(e));
        EXPECT_FALSE(world.Has<Velocity>(e));
        EXPECT_TRUE(world.Has<Health>(e));
    }

    // Performance check
    EXPECT_LT(duration.count(), 1000) << "Archetype migrations took " << duration.count() << "ms";
}

// ===== Multi-threaded Scenarios =====

TEST_F(ECS_Integration, MultiThreaded_ConcurrentEntityCreation)
{
    World world;
    const size_t threadsCount = 4;
    const size_t entitiesPerThread = 1000;

    std::vector<std::thread> threads;
    std::atomic<size_t> successCount{0};

    auto start = std::chrono::high_resolution_clock::now();

    // Note: World is NOT thread-safe for mutations, but this tests that
    // the system doesn't crash under concurrent access (even if some operations fail)
    for (size_t t = 0; t < threadsCount; ++t)
    {
        threads.emplace_back([&world, &successCount, entitiesPerThread]() {
            for (size_t i = 0; i < entitiesPerThread; ++i)
            {
                Entity e = world.CreateEntity();
                if (world.IsAlive(e))
                {
                    Position pos{ 0.0f, 0.0f, 0.0f };
                    if (world.Add(e, pos).has_value())
                    {
                        successCount.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            }
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Note: Due to lack of thread safety, not all operations may succeed
    // But the system should not crash
    EXPECT_GT(successCount.load(), 0u) << "At least some concurrent operations should succeed";
    
    // Performance check
    EXPECT_LT(duration.count(), 2000) << "Concurrent creation took " << duration.count() << "ms";
}

TEST_F(ECS_Integration, MultiThreaded_ParallelQueries_Concurrent)
{
    World world;
    const size_t entityCount = 5000;

    // Create entities
    for (size_t i = 0; i < entityCount; ++i)
    {
        Entity e = world.CreateEntity();
        Position pos{ static_cast<float>(i), 0.0f, 0.0f };
        Velocity vel{ 1.0f, 0.0f, 0.0f };
        world.Add(e, pos);
        world.Add(e, vel);
    }

    auto query = world.QueryComponents<Position, Velocity>();
    auto parallel = MakeParallel(query);

    std::atomic<int> iteration1Count{0};
    std::atomic<int> iteration2Count{0};

    auto start = std::chrono::high_resolution_clock::now();

    // Run two parallel queries concurrently
    std::thread t1([&]() {
        parallel.Execute([&iteration1Count](Position& pos, Velocity& vel) {
            pos.x += vel.dx;
            iteration1Count.fetch_add(1, std::memory_order_relaxed);
        });
    });

    std::thread t2([&]() {
        parallel.Execute([&iteration2Count](Position& pos, Velocity& vel) {
            pos.y += vel.dy;
            iteration2Count.fetch_add(1, std::memory_order_relaxed);
        });
    });

    t1.join();
    t2.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Both queries should process all entities
    EXPECT_EQ(iteration1Count.load(), entityCount);
    EXPECT_EQ(iteration2Count.load(), entityCount);

    // Performance check
    EXPECT_LT(duration.count(), 500) << "Concurrent parallel queries took " << duration.count() << "ms";
}

// ===== Edge Case Tests =====

TEST_F(ECS_Integration, EdgeCase_MaxEntityLimit)
{
    WorldOptions opts;
    opts.maxEntities = 1000;
    World world(opts);

    // Create max entities
    std::vector<Entity> entities;
    entities.reserve(1000);

    for (uint32_t i = 0; i < 1000; ++i)
    {
        Entity e = world.CreateEntity();
        EXPECT_TRUE(world.IsAlive(e));
        entities.push_back(e);
    }

    EXPECT_EQ(world.AliveCount(), 1000u);

    // Try to create one more (should fail)
    Entity overflow = world.CreateEntity();
    EXPECT_FALSE(world.IsAlive(overflow));

    // Destroy one entity
    ASSERT_TRUE(world.DestroyEntity(entities[0]).has_value());
    EXPECT_EQ(world.AliveCount(), 999u);

    // Now we should be able to create one more
    Entity newEntity = world.CreateEntity();
    EXPECT_TRUE(world.IsAlive(newEntity));
    EXPECT_EQ(world.AliveCount(), 1000u);
}

TEST_F(ECS_Integration, EdgeCase_EmptyQuery)
{
    World world;

    // Create entities with Position only
    for (size_t i = 0; i < 100; ++i)
    {
        Entity e = world.CreateEntity();
        Position pos{ 0.0f, 0.0f, 0.0f };
        world.Add(e, pos);
    }

    // Query for Velocity (no entities have it)
    auto query = world.QueryComponents<Velocity>();

    size_t count = 0;
    query.ForEach([&count](Velocity& vel) {
        count++;
    });

    EXPECT_EQ(count, 0u);

    // Parallel query on empty result
    auto parallel = MakeParallel(query);
    std::atomic<size_t> parallelCount{0};

    parallel.Execute([&parallelCount](Velocity& vel) {
        parallelCount.fetch_add(1, std::memory_order_relaxed);
    });

    EXPECT_EQ(parallelCount.load(), 0u);
}

TEST_F(ECS_Integration, EdgeCase_FullChunks_MultipleArchetypes)
{
    World world;

    // Create enough entities to fill multiple chunks
    // Chunk capacity varies by component size, but typically 500-1000 entities per chunk
    const size_t entityCount = 5000;

    // Create entities with different component combinations
    for (size_t i = 0; i < entityCount; ++i)
    {
        Entity e = world.CreateEntity();
        Position pos{ static_cast<float>(i), 0.0f, 0.0f };
        world.Add(e, pos);

        // Every other entity gets Velocity
        if (i % 2 == 0)
        {
            Velocity vel{ 1.0f, 0.0f, 0.0f };
            world.Add(e, vel);
        }

        // Every third entity gets Health
        if (i % 3 == 0)
        {
            Health health{ 100, 100 };
            world.Add(e, health);
        }
    }

    EXPECT_EQ(world.AliveCount(), entityCount);

    // Query each archetype
    auto posQuery = world.QueryComponents<Position>();
    auto posVelQuery = world.QueryComponents<Position, Velocity>();
    auto posHealthQuery = world.QueryComponents<Position, Health>();

    size_t posCount = 0;
    posQuery.ForEach([&posCount](Position& pos) { posCount++; });
    EXPECT_EQ(posCount, entityCount);

    size_t posVelCount = 0;
    posVelQuery.ForEach([&posVelCount](Position& pos, Velocity& vel) { posVelCount++; });
    EXPECT_GT(posVelCount, 0u);

    size_t posHealthCount = 0;
    posHealthQuery.ForEach([&posHealthCount](Position& pos, Health& health) { posHealthCount++; });
    EXPECT_GT(posHealthCount, 0u);
}

TEST_F(ECS_Integration, EdgeCase_WorldClear_LargeScale)
{
    World world;
    const size_t entityCount = 10000;

    // Create many entities
    for (size_t i = 0; i < entityCount; ++i)
    {
        Entity e = world.CreateEntity();
        Position pos{ 0.0f, 0.0f, 0.0f };
        Velocity vel{ 1.0f, 0.0f, 0.0f };
        world.Add(e, pos);
        world.Add(e, vel);
    }

    EXPECT_EQ(world.AliveCount(), entityCount);

    auto start = std::chrono::high_resolution_clock::now();

    // Clear the world
    world.Clear();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Verify world is empty
    EXPECT_EQ(world.AliveCount(), 0u);
    EXPECT_EQ(world.Capacity(), 0u);

    // Should be fast
    EXPECT_LT(duration.count(), 100) << "Clearing 10K entities took " << duration.count() << "ms";

    // Should be able to create new entities
    Entity e = world.CreateEntity();
    EXPECT_TRUE(world.IsAlive(e));
    EXPECT_EQ(e.index, 0u);
    EXPECT_EQ(e.version, 1u);
}

// ===== Complex Scenario Tests =====

TEST_F(ECS_Integration, ComplexScenario_GameSimulation_1000_Frames)
{
    World world;
    const size_t entityCount = 1000;
    const size_t frameCount = 1000;

    // Create game entities
    std::vector<Entity> entities;
    entities.reserve(entityCount);

    for (size_t i = 0; i < entityCount; ++i)
    {
        Entity e = world.CreateEntity();
        entities.push_back(e);

        Position pos{ static_cast<float>(i % 100), static_cast<float>(i / 100), 0.0f };
        Velocity vel{
            static_cast<float>((i % 3) - 1),
            static_cast<float>((i % 5) - 2),
            0.0f
        };
        Health health{ 100, 100 };

        world.Add(e, pos);
        world.Add(e, vel);
        world.Add(e, health);
    }

    auto moveQuery = world.QueryComponents<Position, Velocity>();
    auto moveParallel = MakeParallel(moveQuery);

    auto start = std::chrono::high_resolution_clock::now();

    // Simulate 1000 frames
    for (size_t frame = 0; frame < frameCount; ++frame)
    {
        // Movement system (parallel)
        moveParallel.Execute([](Position& pos, Velocity& vel) {
            pos.x += vel.dx * 0.016f; // 60 FPS
            pos.y += vel.dy * 0.016f;
            pos.z += vel.dz * 0.016f;
        });

        // Damage system (sequential, every 10 frames)
        if (frame % 10 == 0)
        {
            auto healthQuery = world.QueryComponents<Health>();
            healthQuery.ForEach([](Health& health) {
                health.current = std::max(0, health.current - 1);
            });
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Verify simulation ran
    EXPECT_EQ(world.AliveCount(), entityCount);

    // Performance check: 1000 frames should complete in reasonable time
    EXPECT_LT(duration.count(), 5000) << "1000 frame simulation took " << duration.count() << "ms";

    // Calculate FPS
    double fps = (frameCount * 1000.0) / duration.count();
    EXPECT_GT(fps, 200.0) << "Simulation FPS: " << fps;
}


