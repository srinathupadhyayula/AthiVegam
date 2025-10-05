#include <gtest/gtest.h>
#include "ECS/World.hpp"

using namespace Engine::ECS;

TEST(ECS_EntityLifecycle, CreateDestroy_Basic)
{
    World world;
    auto e = world.CreateEntity();
    EXPECT_TRUE(world.IsAlive(e));
    auto res = world.DestroyEntity(e);
    EXPECT_TRUE(res.has_value());
    EXPECT_FALSE(world.IsAlive(e));
}

TEST(ECS_EntityLifecycle, VersionIncrementOnReuse)
{
    World world;
    auto e1 = world.CreateEntity();
    ASSERT_TRUE(world.IsAlive(e1));
    ASSERT_TRUE(world.DestroyEntity(e1).has_value());

    auto e2 = world.CreateEntity();
    EXPECT_EQ(e2.index, e1.index);
    EXPECT_EQ(e2.version, e1.version + 1);
    EXPECT_TRUE(world.IsAlive(e2));
}

TEST(ECS_EntityLifecycle, InvalidEntityAccessRejected)
{
    World world;
    auto e = world.CreateEntity();
    ASSERT_TRUE(world.DestroyEntity(e).has_value());

    auto valid = world.Validate(e);
    EXPECT_FALSE(valid.has_value());
    EXPECT_EQ(valid.error(), Error::InvalidEntity);
}

TEST(ECS_EntityLifecycle, DestroyAlreadyDestroyed)
{
    World world;
    auto e = world.CreateEntity();
    ASSERT_TRUE(world.DestroyEntity(e).has_value());

    auto again = world.DestroyEntity(e);
    EXPECT_FALSE(again.has_value());
    EXPECT_EQ(again.error(), Error::AlreadyDestroyed);
}

TEST(ECS_EntityLifecycle, MaxEntityCount)
{
    WorldOptions opts{}; opts.maxEntities = 2u;
    World world(opts);

    auto e1 = world.CreateEntity();
    auto e2 = world.CreateEntity();
    EXPECT_TRUE(world.IsAlive(e1));
    EXPECT_TRUE(world.IsAlive(e2));

    auto e3 = world.CreateEntity();
    EXPECT_FALSE(world.IsAlive(e3));

    ASSERT_TRUE(world.DestroyEntity(e1).has_value());
    auto e4 = world.CreateEntity();
    EXPECT_TRUE(world.IsAlive(e4));
}

TEST(ECS_EntityLifecycle, Clear_EmptyWorld)
{
    World world;
    EXPECT_EQ(world.AliveCount(), 0u);
    EXPECT_EQ(world.Capacity(), 0u);

    world.Clear();

    EXPECT_EQ(world.AliveCount(), 0u);
    EXPECT_EQ(world.Capacity(), 0u);
}

TEST(ECS_EntityLifecycle, Clear_WithEntities)
{
    World world;

    // Create multiple entities
    auto e1 = world.CreateEntity();
    auto e2 = world.CreateEntity();
    auto e3 = world.CreateEntity();

    EXPECT_EQ(world.AliveCount(), 3u);
    EXPECT_TRUE(world.IsAlive(e1));
    EXPECT_TRUE(world.IsAlive(e2));
    EXPECT_TRUE(world.IsAlive(e3));

    // Clear the world
    world.Clear();

    // Verify all state is reset
    EXPECT_EQ(world.AliveCount(), 0u);
    EXPECT_EQ(world.Capacity(), 0u);
    EXPECT_FALSE(world.IsAlive(e1));
    EXPECT_FALSE(world.IsAlive(e2));
    EXPECT_FALSE(world.IsAlive(e3));
}

TEST(ECS_EntityLifecycle, Clear_PreservesOptions)
{
    WorldOptions opts{};
    opts.maxEntities = 10u;
    World world(opts);

    // Create some entities
    auto e1 = world.CreateEntity();
    auto e2 = world.CreateEntity();
    EXPECT_EQ(world.AliveCount(), 2u);

    // Clear the world
    world.Clear();

    // Verify options are preserved - should still be able to create up to 10 entities
    for (uint32_t i = 0; i < 10; ++i)
    {
        auto e = world.CreateEntity();
        EXPECT_TRUE(world.IsAlive(e));
    }
    EXPECT_EQ(world.AliveCount(), 10u);

    // 11th entity should fail
    auto e11 = world.CreateEntity();
    EXPECT_FALSE(world.IsAlive(e11));
}

TEST(ECS_EntityLifecycle, Clear_AfterReuse)
{
    World world;

    // Create and destroy entities to populate free list
    auto e1 = world.CreateEntity();
    auto e2 = world.CreateEntity();
    ASSERT_TRUE(world.DestroyEntity(e1).has_value());
    ASSERT_TRUE(world.DestroyEntity(e2).has_value());

    // Create new entities (should reuse indices)
    auto e3 = world.CreateEntity();
    auto e4 = world.CreateEntity();
    EXPECT_EQ(world.AliveCount(), 2u);

    // Clear should reset everything including free list
    world.Clear();
    EXPECT_EQ(world.AliveCount(), 0u);
    EXPECT_EQ(world.Capacity(), 0u);

    // New entities should start from index 0 again
    auto e5 = world.CreateEntity();
    EXPECT_EQ(e5.index, 0u);
    EXPECT_EQ(e5.version, 1u);
}

TEST(ECS_EntityLifecycle, GetEntityInfo_InvalidEntity)
{
    World world;
    Entity invalid{ 999, 1 };

    auto result = world.GetEntityInfo(invalid);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), Error::InvalidEntity);
}

TEST(ECS_EntityLifecycle, GetEntityInfo_DestroyedEntity)
{
    World world;
    auto e = world.CreateEntity();
    ASSERT_TRUE(world.DestroyEntity(e).has_value());

    auto result = world.GetEntityInfo(e);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), Error::InvalidEntity);
}

TEST(ECS_EntityLifecycle, GetEntityInfo_ValidEntity)
{
    World world;
    auto e = world.CreateEntity();

    auto result = world.GetEntityInfo(e);
    ASSERT_TRUE(result.has_value());

    const auto& info = result.value();
    EXPECT_TRUE(info.isAlive);
    // Entity without components should have null signature
    EXPECT_EQ(info.signature, nullptr);
}

