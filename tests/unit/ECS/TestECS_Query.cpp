#include <gtest/gtest.h>
#include "ECS/World.hpp"
#include "ECS/ComponentRegistry.hpp"

using namespace Engine::ECS;

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

// Register components
REGISTER_COMPONENT(Position)
REGISTER_COMPONENT(Velocity)
REGISTER_COMPONENT(Health)
REGISTER_COMPONENT(Tag)

// ===== Basic Query Tests =====

TEST(ECS_Query, SingleComponent_EmptyWorld)
{
    World world;
    auto query = world.QueryComponents<Position>();
    
    EXPECT_TRUE(query.Empty());
    EXPECT_EQ(query.EntityCount(), 0);
    EXPECT_EQ(query.ChunkCount(), 0);
}

TEST(ECS_Query, SingleComponent_Basic)
{
    World world;
    
    // Create entities with Position
    auto e1 = world.CreateEntity();
    auto e2 = world.CreateEntity();
    auto e3 = world.CreateEntity();
    
    world.Add(e1, Position{ 1.0f, 2.0f, 3.0f });
    world.Add(e2, Position{ 4.0f, 5.0f, 6.0f });
    world.Add(e3, Position{ 7.0f, 8.0f, 9.0f });
    
    auto query = world.QueryComponents<Position>();
    
    EXPECT_FALSE(query.Empty());
    EXPECT_EQ(query.EntityCount(), 3);
    
    // Verify we can iterate
    size_t count = 0;
    for (auto it = query.begin(); it != query.end(); ++it)
    {
        auto [positions] = *it;
        EXPECT_NE(positions, nullptr);
        count += it.Count();
    }
    EXPECT_EQ(count, 3);
}

TEST(ECS_Query, MultipleComponents_Basic)
{
    World world;
    
    // Create entities with Position + Velocity
    auto e1 = world.CreateEntity();
    auto e2 = world.CreateEntity();
    
    world.Add(e1, Position{ 1.0f, 2.0f, 3.0f });
    world.Add(e1, Velocity{ 0.1f, 0.2f, 0.3f });
    
    world.Add(e2, Position{ 4.0f, 5.0f, 6.0f });
    world.Add(e2, Velocity{ 0.4f, 0.5f, 0.6f });
    
    // Create entity with only Position (should not match)
    auto e3 = world.CreateEntity();
    world.Add(e3, Position{ 7.0f, 8.0f, 9.0f });
    
    auto query = world.QueryComponents<Position, Velocity>();
    
    EXPECT_FALSE(query.Empty());
    EXPECT_EQ(query.EntityCount(), 2);
    
    // Verify component access
    size_t count = 0;
    for (auto it = query.begin(); it != query.end(); ++it)
    {
        auto [positions, velocities] = *it;
        EXPECT_NE(positions, nullptr);
        EXPECT_NE(velocities, nullptr);
        count += it.Count();
    }
    EXPECT_EQ(count, 2);
}

TEST(ECS_Query, ExcludeComponents_Basic)
{
    World world;
    
    // Create entities with Position
    auto e1 = world.CreateEntity();
    auto e2 = world.CreateEntity();
    auto e3 = world.CreateEntity();
    
    world.Add(e1, Position{ 1.0f, 2.0f, 3.0f });
    
    world.Add(e2, Position{ 4.0f, 5.0f, 6.0f });
    world.Add(e2, Tag{ 42 }); // This one has a tag
    
    world.Add(e3, Position{ 7.0f, 8.0f, 9.0f });
    
    // Query for Position but exclude Tag
    auto query = world.QueryComponents<Position>(Exclude<Tag>{});
    
    EXPECT_FALSE(query.Empty());
    EXPECT_EQ(query.EntityCount(), 2); // e1 and e3, not e2
}

TEST(ECS_Query, ExcludeMultipleComponents)
{
    World world;
    
    auto e1 = world.CreateEntity();
    auto e2 = world.CreateEntity();
    auto e3 = world.CreateEntity();
    auto e4 = world.CreateEntity();
    
    world.Add(e1, Position{ 1.0f, 2.0f, 3.0f });
    
    world.Add(e2, Position{ 4.0f, 5.0f, 6.0f });
    world.Add(e2, Tag{ 42 });
    
    world.Add(e3, Position{ 7.0f, 8.0f, 9.0f });
    world.Add(e3, Health{ 100, 100 });
    
    world.Add(e4, Position{ 10.0f, 11.0f, 12.0f });
    
    // Query for Position but exclude both Tag and Health
    auto query = world.QueryComponents<Position>(Exclude<Tag, Health>{});
    
    EXPECT_EQ(query.EntityCount(), 2); // e1 and e4
}

// ===== Iteration Tests =====

TEST(ECS_Query, ForEach_Basic)
{
    World world;
    
    auto e1 = world.CreateEntity();
    auto e2 = world.CreateEntity();
    auto e3 = world.CreateEntity();
    
    world.Add(e1, Position{ 1.0f, 0.0f, 0.0f });
    world.Add(e2, Position{ 2.0f, 0.0f, 0.0f });
    world.Add(e3, Position{ 3.0f, 0.0f, 0.0f });
    
    auto query = world.QueryComponents<Position>();
    
    float sum = 0.0f;
    query.ForEach([&sum](Position& pos) {
        sum += pos.x;
    });
    
    EXPECT_FLOAT_EQ(sum, 6.0f); // 1 + 2 + 3
}

TEST(ECS_Query, ForEach_MultipleComponents)
{
    World world;
    
    auto e1 = world.CreateEntity();
    auto e2 = world.CreateEntity();
    
    world.Add(e1, Position{ 10.0f, 20.0f, 30.0f });
    world.Add(e1, Velocity{ 1.0f, 2.0f, 3.0f });
    
    world.Add(e2, Position{ 40.0f, 50.0f, 60.0f });
    world.Add(e2, Velocity{ 4.0f, 5.0f, 6.0f });
    
    auto query = world.QueryComponents<Position, Velocity>();
    
    // Apply velocity to position
    query.ForEach([](Position& pos, Velocity& vel) {
        pos.x += vel.dx;
        pos.y += vel.dy;
        pos.z += vel.dz;
    });
    
    // Verify positions were updated
    auto pos1 = world.Get<Position>(e1);
    ASSERT_TRUE(pos1.has_value());
    EXPECT_FLOAT_EQ((*pos1)->x, 11.0f);
    EXPECT_FLOAT_EQ((*pos1)->y, 22.0f);
    EXPECT_FLOAT_EQ((*pos1)->z, 33.0f);
    
    auto pos2 = world.Get<Position>(e2);
    ASSERT_TRUE(pos2.has_value());
    EXPECT_FLOAT_EQ((*pos2)->x, 44.0f);
    EXPECT_FLOAT_EQ((*pos2)->y, 55.0f);
    EXPECT_FLOAT_EQ((*pos2)->z, 66.0f);
}

TEST(ECS_Query, ManualIteration_ChunkLevel)
{
    World world;
    
    // Create several entities
    for (int i = 0; i < 10; ++i)
    {
        auto e = world.CreateEntity();
        world.Add(e, Position{ static_cast<float>(i), 0.0f, 0.0f });
    }
    
    auto query = world.QueryComponents<Position>();
    
    size_t totalEntities = 0;
    size_t chunkCount = 0;
    
    for (auto it = query.begin(); it != query.end(); ++it)
    {
        auto [positions] = *it;
        size_t count = it.Count();
        
        EXPECT_NE(positions, nullptr);
        EXPECT_GT(count, 0);
        
        totalEntities += count;
        ++chunkCount;
    }
    
    EXPECT_EQ(totalEntities, 10);
    EXPECT_GT(chunkCount, 0);
}

// ===== Dynamic Query Tests =====

TEST(ECS_Query, QueryAfterAddComponent)
{
    World world;
    
    auto e1 = world.CreateEntity();
    world.Add(e1, Position{ 1.0f, 2.0f, 3.0f });
    
    auto query1 = world.QueryComponents<Position, Velocity>();
    EXPECT_EQ(query1.EntityCount(), 0);
    
    // Add Velocity component
    world.Add(e1, Velocity{ 0.1f, 0.2f, 0.3f });
    
    // Create new query (old query won't update)
    auto query2 = world.QueryComponents<Position, Velocity>();
    EXPECT_EQ(query2.EntityCount(), 1);
}

TEST(ECS_Query, QueryAfterRemoveComponent)
{
    World world;
    
    auto e1 = world.CreateEntity();
    world.Add(e1, Position{ 1.0f, 2.0f, 3.0f });
    world.Add(e1, Velocity{ 0.1f, 0.2f, 0.3f });
    
    auto query1 = world.QueryComponents<Position, Velocity>();
    EXPECT_EQ(query1.EntityCount(), 1);
    
    // Remove Velocity component
    world.Remove<Velocity>(e1);
    
    // Create new query
    auto query2 = world.QueryComponents<Position, Velocity>();
    EXPECT_EQ(query2.EntityCount(), 0);
    
    // Position-only query should still find it
    auto query3 = world.QueryComponents<Position>();
    EXPECT_EQ(query3.EntityCount(), 1);
}

TEST(ECS_Query, QueryAfterDestroyEntity)
{
    World world;
    
    auto e1 = world.CreateEntity();
    auto e2 = world.CreateEntity();
    
    world.Add(e1, Position{ 1.0f, 2.0f, 3.0f });
    world.Add(e2, Position{ 4.0f, 5.0f, 6.0f });
    
    auto query1 = world.QueryComponents<Position>();
    EXPECT_EQ(query1.EntityCount(), 2);
    
    // Destroy one entity
    world.DestroyEntity(e1);
    
    // Create new query
    auto query2 = world.QueryComponents<Position>();
    EXPECT_EQ(query2.EntityCount(), 1);
}

// ===== Edge Cases =====

TEST(ECS_Query, EmptyQuery_NoMatchingArchetypes)
{
    World world;
    
    // Create entities with Position
    auto e1 = world.CreateEntity();
    world.Add(e1, Position{ 1.0f, 2.0f, 3.0f });
    
    // Query for Health (no entities have it)
    auto query = world.QueryComponents<Health>();
    
    EXPECT_TRUE(query.Empty());
    EXPECT_EQ(query.EntityCount(), 0);
    
    // ForEach should not execute
    bool executed = false;
    query.ForEach([&executed](Health&) {
        executed = true;
    });
    EXPECT_FALSE(executed);
}

TEST(ECS_Query, MixedArchetypes)
{
    World world;
    
    // Create entities with different component combinations
    auto e1 = world.CreateEntity();
    world.Add(e1, Position{ 1.0f, 2.0f, 3.0f });
    
    auto e2 = world.CreateEntity();
    world.Add(e2, Position{ 4.0f, 5.0f, 6.0f });
    world.Add(e2, Velocity{ 0.1f, 0.2f, 0.3f });
    
    auto e3 = world.CreateEntity();
    world.Add(e3, Position{ 7.0f, 8.0f, 9.0f });
    world.Add(e3, Velocity{ 0.4f, 0.5f, 0.6f });
    world.Add(e3, Health{ 100, 100 });
    
    // Query for Position should find all 3
    auto query1 = world.QueryComponents<Position>();
    EXPECT_EQ(query1.EntityCount(), 3);
    
    // Query for Position + Velocity should find e2 and e3
    auto query2 = world.QueryComponents<Position, Velocity>();
    EXPECT_EQ(query2.EntityCount(), 2);
    
    // Query for all three should find only e3
    auto query3 = world.QueryComponents<Position, Velocity, Health>();
    EXPECT_EQ(query3.EntityCount(), 1);
}

