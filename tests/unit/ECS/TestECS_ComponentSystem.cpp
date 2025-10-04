#include <gtest/gtest.h>
#include "ECS/World.hpp"
#include "ECS/ComponentTraits.hpp"
#include "ECS/ComponentRegistry.hpp"

using namespace Engine::ECS;

// Test component types
struct Position {
    float x, y, z;
};

struct Velocity {
    float dx, dy, dz;
};

struct Health {
    int32_t current, maximum;
};

// Register components
REGISTER_COMPONENT(Position)
REGISTER_COMPONENT(Velocity)
REGISTER_COMPONENT(Health)

// Component concept tests
TEST(ECS_ComponentSystem, ComponentConceptValidation)
{
    // These should compile (satisfy Component concept)
    static_assert(Component<Position>);
    static_assert(Component<Velocity>);
    static_assert(Component<Health>);
    static_assert(Component<int>);
    static_assert(Component<float>);
}

TEST(ECS_ComponentSystem, ComponentTypeIDUniqueness)
{
    auto posID = GetComponentTypeID<Position>();
    auto velID = GetComponentTypeID<Velocity>();
    auto healthID = GetComponentTypeID<Health>();

    EXPECT_NE(posID, velID);
    EXPECT_NE(posID, healthID);
    EXPECT_NE(velID, healthID);

    // Same type should give same ID
    EXPECT_EQ(posID, GetComponentTypeID<Position>());
}

TEST(ECS_ComponentSystem, ComponentSignatureBasics)
{
    ComponentSignature sig;
    EXPECT_TRUE(sig.Empty());
    EXPECT_EQ(sig.Count(), 0u);

    sig.Add<Position>();
    EXPECT_FALSE(sig.Empty());
    EXPECT_EQ(sig.Count(), 1u);
    EXPECT_TRUE(sig.Contains<Position>());
    EXPECT_FALSE(sig.Contains<Velocity>());

    sig.Add<Velocity>();
    EXPECT_EQ(sig.Count(), 2u);
    EXPECT_TRUE(sig.Contains<Position>());
    EXPECT_TRUE(sig.Contains<Velocity>());
}

TEST(ECS_ComponentSystem, ComponentSignatureRemove)
{
    ComponentSignature sig;
    sig.Add<Position>();
    sig.Add<Velocity>();
    sig.Add<Health>();
    EXPECT_EQ(sig.Count(), 3u);

    sig.Remove<Velocity>();
    EXPECT_EQ(sig.Count(), 2u);
    EXPECT_TRUE(sig.Contains<Position>());
    EXPECT_FALSE(sig.Contains<Velocity>());
    EXPECT_TRUE(sig.Contains<Health>());
}

TEST(ECS_ComponentSystem, ComponentSignatureEquality)
{
    ComponentSignature sig1, sig2;
    
    sig1.Add<Position>();
    sig1.Add<Velocity>();
    
    sig2.Add<Velocity>();
    sig2.Add<Position>();

    // Order shouldn't matter (signatures are sorted)
    EXPECT_EQ(sig1, sig2);

    sig2.Add<Health>();
    EXPECT_NE(sig1, sig2);
}

TEST(ECS_ComponentSystem, ComponentSignatureHashing)
{
    ComponentSignature sig1, sig2;
    
    sig1.Add<Position>();
    sig1.Add<Velocity>();
    
    sig2.Add<Position>();
    sig2.Add<Velocity>();

    // Same signature should have same hash
    EXPECT_EQ(sig1.Hash(), sig2.Hash());

    sig2.Add<Health>();
    // Different signature should (likely) have different hash
    EXPECT_NE(sig1.Hash(), sig2.Hash());
}

TEST(ECS_ComponentSystem, ComponentRegistryBasics)
{
    auto& registry = ComponentRegistry::Instance();
    
    // Components should be auto-registered via REGISTER_COMPONENT macro
    auto* posMeta = registry.GetMetadata<Position>();
    ASSERT_NE(posMeta, nullptr);
    EXPECT_EQ(posMeta->size, sizeof(Position));
    EXPECT_EQ(posMeta->alignment, alignof(Position));

    auto* velMeta = registry.GetMetadata<Velocity>();
    ASSERT_NE(velMeta, nullptr);
    EXPECT_EQ(velMeta->size, sizeof(Velocity));
}

