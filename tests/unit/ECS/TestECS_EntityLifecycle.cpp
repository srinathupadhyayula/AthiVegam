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

