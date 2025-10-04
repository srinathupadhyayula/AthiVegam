#include <gtest/gtest.h>
#include "ECS/World.hpp"

using namespace Engine::ECS;

// Test component types
struct Transform {
    float x, y, z;
    float rotX, rotY, rotZ;
};

struct RigidBody {
    float mass;
    float drag;
};

struct Tag {
    uint32_t value;
};

// Register components
REGISTER_COMPONENT(Transform)
REGISTER_COMPONENT(RigidBody)
REGISTER_COMPONENT(Tag)

TEST(ECS_ComponentOperations, AddComponent_Basic)
{
    World world;
    auto e = world.CreateEntity();
    ASSERT_TRUE(world.IsAlive(e));

    Transform transform{ 1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 0.0f };
    auto result = world.Add(e, transform);
    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(world.Has<Transform>(e));
}

TEST(ECS_ComponentOperations, AddComponent_InvalidEntity)
{
    World world;
    Entity invalid{ 999, 1 };

    Transform transform{};
    auto result = world.Add(invalid, transform);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), Error::InvalidEntity);
}

TEST(ECS_ComponentOperations, AddComponent_AlreadyExists)
{
    World world;
    auto e = world.CreateEntity();

    Transform transform1{ 1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 0.0f };
    ASSERT_TRUE(world.Add(e, transform1).has_value());

    Transform transform2{ 4.0f, 5.0f, 6.0f, 0.0f, 0.0f, 0.0f };
    auto result = world.Add(e, transform2);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), Error::ComponentAlreadyExists);
}

TEST(ECS_ComponentOperations, GetComponent_Basic)
{
    World world;
    auto e = world.CreateEntity();

    Transform transform{ 1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 0.0f };
    ASSERT_TRUE(world.Add(e, transform).has_value());

    auto result = world.Get<Transform>(e);
    ASSERT_TRUE(result.has_value());
    
    Transform* retrieved = *result;
    ASSERT_NE(retrieved, nullptr);
    EXPECT_FLOAT_EQ(retrieved->x, 1.0f);
    EXPECT_FLOAT_EQ(retrieved->y, 2.0f);
    EXPECT_FLOAT_EQ(retrieved->z, 3.0f);
}

TEST(ECS_ComponentOperations, GetComponent_NotFound)
{
    World world;
    auto e = world.CreateEntity();

    auto result = world.Get<Transform>(e);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), Error::ComponentNotFound);
}

TEST(ECS_ComponentOperations, GetComponent_Mutable)
{
    World world;
    auto e = world.CreateEntity();

    Transform transform{ 1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 0.0f };
    ASSERT_TRUE(world.Add(e, transform).has_value());

    auto result = world.Get<Transform>(e);
    ASSERT_TRUE(result.has_value());
    
    // Modify component
    (*result)->x = 10.0f;
    (*result)->y = 20.0f;

    // Verify modification persisted
    auto result2 = world.Get<Transform>(e);
    ASSERT_TRUE(result2.has_value());
    EXPECT_FLOAT_EQ((*result2)->x, 10.0f);
    EXPECT_FLOAT_EQ((*result2)->y, 20.0f);
}

TEST(ECS_ComponentOperations, RemoveComponent_Basic)
{
    World world;
    auto e = world.CreateEntity();

    Transform transform{ 1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 0.0f };
    ASSERT_TRUE(world.Add(e, transform).has_value());
    ASSERT_TRUE(world.Has<Transform>(e));

    auto result = world.Remove<Transform>(e);
    EXPECT_TRUE(result.has_value());
    EXPECT_FALSE(world.Has<Transform>(e));
}

TEST(ECS_ComponentOperations, RemoveComponent_NotFound)
{
    World world;
    auto e = world.CreateEntity();

    auto result = world.Remove<Transform>(e);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), Error::ComponentNotFound);
}

TEST(ECS_ComponentOperations, MultipleComponents)
{
    World world;
    auto e = world.CreateEntity();

    Transform transform{ 1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 0.0f };
    RigidBody body{ 10.0f, 0.5f };
    Tag tag{ 42 };

    ASSERT_TRUE(world.Add(e, transform).has_value());
    ASSERT_TRUE(world.Add(e, body).has_value());
    ASSERT_TRUE(world.Add(e, tag).has_value());

    EXPECT_TRUE(world.Has<Transform>(e));
    EXPECT_TRUE(world.Has<RigidBody>(e));
    EXPECT_TRUE(world.Has<Tag>(e));

    auto transformResult = world.Get<Transform>(e);
    auto bodyResult = world.Get<RigidBody>(e);
    auto tagResult = world.Get<Tag>(e);

    ASSERT_TRUE(transformResult.has_value());
    ASSERT_TRUE(bodyResult.has_value());
    ASSERT_TRUE(tagResult.has_value());

    EXPECT_FLOAT_EQ((*transformResult)->x, 1.0f);
    EXPECT_FLOAT_EQ((*bodyResult)->mass, 10.0f);
    EXPECT_EQ((*tagResult)->value, 42u);
}

TEST(ECS_ComponentOperations, ArchetypeMigration_AddComponent)
{
    World world;
    auto e = world.CreateEntity();

    // Start with Transform only
    Transform transform{ 1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 0.0f };
    ASSERT_TRUE(world.Add(e, transform).has_value());

    // Add RigidBody - should migrate to new archetype
    RigidBody body{ 10.0f, 0.5f };
    ASSERT_TRUE(world.Add(e, body).has_value());

    // Verify both components present and data preserved
    auto transformResult = world.Get<Transform>(e);
    auto bodyResult = world.Get<RigidBody>(e);

    ASSERT_TRUE(transformResult.has_value());
    ASSERT_TRUE(bodyResult.has_value());

    EXPECT_FLOAT_EQ((*transformResult)->x, 1.0f);
    EXPECT_FLOAT_EQ((*transformResult)->y, 2.0f);
    EXPECT_FLOAT_EQ((*bodyResult)->mass, 10.0f);
}

TEST(ECS_ComponentOperations, ArchetypeMigration_RemoveComponent)
{
    World world;
    auto e = world.CreateEntity();

    // Add multiple components
    Transform transform{ 1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 0.0f };
    RigidBody body{ 10.0f, 0.5f };
    ASSERT_TRUE(world.Add(e, transform).has_value());
    ASSERT_TRUE(world.Add(e, body).has_value());

    // Remove one component - should migrate to new archetype
    ASSERT_TRUE(world.Remove<RigidBody>(e).has_value());

    // Verify Transform still present with correct data
    EXPECT_TRUE(world.Has<Transform>(e));
    EXPECT_FALSE(world.Has<RigidBody>(e));

    auto transformResult = world.Get<Transform>(e);
    ASSERT_TRUE(transformResult.has_value());
    EXPECT_FLOAT_EQ((*transformResult)->x, 1.0f);
    EXPECT_FLOAT_EQ((*transformResult)->y, 2.0f);
}

TEST(ECS_ComponentOperations, HasComponent)
{
    World world;
    auto e = world.CreateEntity();

    EXPECT_FALSE(world.Has<Transform>(e));

    Transform transform{};
    ASSERT_TRUE(world.Add(e, transform).has_value());
    EXPECT_TRUE(world.Has<Transform>(e));

    ASSERT_TRUE(world.Remove<Transform>(e).has_value());
    EXPECT_FALSE(world.Has<Transform>(e));
}

