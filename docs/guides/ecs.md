# AthiVegam ECS Guide

**Version:** 1.0  
**Last Updated:** 2025-10-05  

---

## Table of Contents

1. [Introduction](#introduction)
2. [Core Concepts](#core-concepts)
3. [Getting Started](#getting-started)
4. [Component Management](#component-management)
5. [Querying Entities](#querying-entities)
6. [Parallel Iteration](#parallel-iteration)
7. [Best Practices](#best-practices)
8. [Performance Tips](#performance-tips)
9. [Error Handling](#error-handling)
10. [Advanced Topics](#advanced-topics)

---

## Introduction

The AthiVegam ECS (Entity Component System) is a high-performance, data-oriented architecture for game entities. It uses archetype-based storage with chunk allocation for cache-friendly iteration and supports parallel processing through the Jobs system.

### Key Features

- **Archetype-based storage:** Entities with the same component set share memory layout
- **Chunk allocation:** 64KB chunks with Structure of Arrays (SoA) layout
- **Type-safe queries:** Compile-time component type checking
- **Parallel iteration:** Automatic parallelization using the Jobs system
- **Version-based entity IDs:** Safe entity handles with version checking
- **Error handling:** `std::expected` for explicit error propagation

---

## Core Concepts

### Entities

Entities are lightweight identifiers composed of a 32-bit index and 32-bit version:

```cpp
struct Entity {
    uint32_t index;    // Index into entity storage
    uint32_t version;  // Version for handle validation
};
```

**Key Points:**
- Entities are just IDs - they don't store data directly
- Version increments on destruction to invalidate old handles
- Free-list reuse for efficient memory management

### Components

Components are plain data structures that define entity behavior:

```cpp
struct Transform {
    float x, y, z;
    float rx, ry, rz;
};

struct Velocity {
    float dx, dy, dz;
};

struct RigidBody {
    float mass;
    float friction;
};
```

**Requirements:**
- Must satisfy `Component` concept (trivially copyable recommended)
- Automatically registered on first use
- No inheritance or virtual functions

### Archetypes

Archetypes represent unique component combinations. Entities with the same components share an archetype:

```
Archetype 1: [Transform]
Archetype 2: [Transform, Velocity]
Archetype 3: [Transform, Velocity, RigidBody]
```

**Benefits:**
- Cache-friendly iteration (all entities with same layout together)
- Efficient component access (no indirection)
- Automatic memory management

### Chunks

Chunks are 64KB memory blocks storing entities in Structure of Arrays (SoA) layout:

```
Chunk Layout:
[Entity Indices][Transform Column][Velocity Column][RigidBody Column]
```

**Characteristics:**
- 64-byte aligned for SIMD operations
- Capacity calculated based on component sizes
- Automatic allocation when full

---

## Getting Started

### Creating a World

```cpp
#include "ECS/World.hpp"

using namespace Engine::ECS;

// Create unbounded world
World world;

// Create world with max entity limit
WorldOptions opts;
opts.maxEntities = 10000;
World limitedWorld(opts);
```

### Creating Entities

```cpp
// Create entity
Entity player = world.CreateEntity();

// Check if entity is alive
if (world.IsAlive(player)) {
    // Entity is valid
}

// Validate entity (returns std::expected)
auto result = world.Validate(player);
if (result.has_value()) {
    // Entity is valid
}
```

### Adding Components

```cpp
Transform transform{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
Velocity velocity{ 1.0f, 0.0f, 0.0f };

// Add components (returns std::expected)
auto addResult = world.Add(player, transform);
if (!addResult.has_value()) {
    // Handle error (e.g., ComponentAlreadyExists)
}

world.Add(player, velocity);
```

### Accessing Components

```cpp
// Get component (returns std::expected<T*, Error>)
auto transformResult = world.Get<Transform>(player);
if (transformResult.has_value()) {
    Transform* t = transformResult.value();
    t->x += 1.0f;
}

// Check if entity has component
if (world.Has<Transform>(player)) {
    // Entity has Transform component
}
```

### Removing Components

```cpp
// Remove component (returns std::expected)
auto removeResult = world.Remove<Velocity>(player);
if (!removeResult.has_value()) {
    // Handle error (e.g., ComponentNotFound)
}
```

### Destroying Entities

```cpp
// Destroy entity (returns std::expected)
auto destroyResult = world.DestroyEntity(player);
if (!destroyResult.has_value()) {
    // Handle error (e.g., AlreadyDestroyed)
}
```

---

## Component Management

### Component Lifecycle

1. **Add:** Entity migrates to new archetype with component
2. **Remove:** Entity migrates to archetype without component
3. **Destroy:** Entity removed from chunk, index freed for reuse

### Archetype Migration

When adding/removing components, entities automatically migrate between archetypes:

```cpp
Entity e = world.CreateEntity();

// Entity in archetype []
world.Add(e, Transform{});
// Entity migrated to archetype [Transform]

world.Add(e, Velocity{});
// Entity migrated to archetype [Transform, Velocity]

world.Remove<Transform>(e);
// Entity migrated to archetype [Velocity]
```

**Important:** Component data is preserved during migration.

---

## Querying Entities

### Basic Queries

```cpp
// Query entities with Transform component
auto query = world.QueryComponents<Transform>();

// Iterate over entities
query.ForEach([](Transform& transform) {
    transform.x += 1.0f;
});
```

### Multi-Component Queries

```cpp
// Query entities with Transform AND Velocity
auto query = world.QueryComponents<Transform, Velocity>();

query.ForEach([](Transform& transform, Velocity& velocity) {
    transform.x += velocity.dx;
    transform.y += velocity.dy;
    transform.z += velocity.dz;
});
```

### Exclude Queries

```cpp
// Query entities with Transform but WITHOUT RigidBody
auto query = world.QueryComponents<Transform>(Exclude<RigidBody>{});

query.ForEach([](Transform& transform) {
    // Process only non-physics entities
});
```

### Chunk-Level Iteration

```cpp
auto query = world.QueryComponents<Transform, Velocity>();

for (auto it = query.begin(); it != query.end(); ++it) {
    auto [transforms, velocities] = *it;
    size_t count = it.Count();
    
    // Process entire chunk at once
    for (size_t i = 0; i < count; ++i) {
        transforms[i].x += velocities[i].dx;
    }
}
```

---

## Parallel Iteration

### Basic Parallel Execution

```cpp
#include "ECS/ParallelQuery.hpp"

auto query = world.QueryComponents<Transform, Velocity>();
auto parallel = MakeParallel(query);

// Execute in parallel using Jobs system
parallel.Execute([](Transform& transform, Velocity& velocity) {
    transform.x += velocity.dx;
    transform.y += velocity.dy;
    transform.z += velocity.dz;
});
```

### Chunk-Level Parallel Execution

```cpp
auto parallel = MakeParallel(query);

parallel.ExecuteChunks([](Transform* transforms, Velocity* velocities, size_t count) {
    // Process entire chunk
    for (size_t i = 0; i < count; ++i) {
        transforms[i].x += velocities[i].dx;
    }
});
```

### Thread-Safe Operations

```cpp
#include <atomic>

std::atomic<int> totalEntities{0};

parallel.Execute([&totalEntities](Transform& transform) {
    totalEntities.fetch_add(1, std::memory_order_relaxed);
});
```

**Important:** Use atomics or per-thread accumulators for shared state.

---

## Best Practices

### 1. Keep Components Small

```cpp
// Good: Small, focused components
struct Position { float x, y, z; };
struct Velocity { float dx, dy, dz; };

// Avoid: Large, monolithic components
struct GameObject {
    Position pos;
    Velocity vel;
    Mesh mesh;
    Material material;
    // ... many more fields
};
```

### 2. Use Queries Efficiently

```cpp
// Good: Query once, iterate many times
auto query = world.QueryComponents<Transform, Velocity>();
for (int frame = 0; frame < 1000; ++frame) {
    query.ForEach([](Transform& t, Velocity& v) {
        t.x += v.dx;
    });
}

// Avoid: Creating query every frame
for (int frame = 0; frame < 1000; ++frame) {
    auto query = world.QueryComponents<Transform, Velocity>();
    query.ForEach([](Transform& t, Velocity& v) {
        t.x += v.dx;
    });
}
```

### 3. Prefer Parallel Iteration

```cpp
// Good: Use parallel iteration for large datasets
auto parallel = MakeParallel(query);
parallel.Execute([](Transform& t, Velocity& v) {
    t.x += v.dx;
});

// Sequential is fine for small datasets (<100 entities)
query.ForEach([](Transform& t) {
    // Simple operation
});
```

### 4. Handle Errors Explicitly

```cpp
// Good: Check error results
auto result = world.Add(entity, component);
if (!result.has_value()) {
    switch (result.error()) {
        case Error::InvalidEntity:
            LOG_ERROR("Entity is invalid");
            break;
        case Error::ComponentAlreadyExists:
            LOG_WARN("Component already exists");
            break;
    }
}

// Avoid: Ignoring errors
world.Add(entity, component); // Error silently ignored
```

---

## Performance Tips

### 1. Batch Entity Creation

```cpp
// Good: Create entities in batch
std::vector<Entity> entities;
entities.reserve(1000);
for (int i = 0; i < 1000; ++i) {
    entities.push_back(world.CreateEntity());
}

// Then add components
for (auto e : entities) {
    world.Add(e, Transform{});
}
```

### 2. Use Chunk-Level Iteration for SIMD

```cpp
// Chunk-level iteration enables SIMD optimization
parallel.ExecuteChunks([](Transform* transforms, Velocity* velocities, size_t count) {
    // Compiler can auto-vectorize this loop
    for (size_t i = 0; i < count; ++i) {
        transforms[i].x += velocities[i].dx;
        transforms[i].y += velocities[i].dy;
        transforms[i].z += velocities[i].dz;
    }
});
```

### 3. Minimize Archetype Migrations

```cpp
// Good: Add all components at once
Entity e = world.CreateEntity();
world.Add(e, Transform{});
world.Add(e, Velocity{});
world.Add(e, RigidBody{});

// Avoid: Frequent add/remove cycles
for (int i = 0; i < 1000; ++i) {
    world.Add(e, SomeComponent{});
    world.Remove<SomeComponent>(e);
}
```

---

## Error Handling

### Error Codes

```cpp
enum class Error {
    InvalidEntity,          // Entity doesn't exist or version mismatch
    AlreadyDestroyed,       // Entity was already destroyed
    EntityLimitReached,     // Max entity count reached
    ComponentNotFound,      // Component doesn't exist on entity
    ComponentAlreadyExists  // Component already exists on entity
};
```

### Checking Errors

```cpp
auto result = world.Add(entity, component);
if (!result.has_value()) {
    Error error = result.error();
    // Handle error
}
```

---

## Advanced Topics

### Entity Metadata Inspection

```cpp
auto info = world.GetEntityInfo(entity);
if (info.has_value()) {
    const auto& metadata = info.value();
    
    if (metadata.signature) {
        // Check components
        bool hasTransform = metadata.signature->Contains<Transform>();
    }
    
    // Get chunk location
    uint32_t chunkIdx = metadata.chunkIndex;
    uint32_t indexInChunk = metadata.indexInChunk;
}
```

### World Management

```cpp
// Clear all entities and reset state
world.Clear();

// Get entity counts
uint32_t alive = world.AliveCount();
uint32_t capacity = world.Capacity();
```

---

## Example: Complete Game Loop

```cpp
#include "ECS/World.hpp"
#include "ECS/ParallelQuery.hpp"

using namespace Engine::ECS;

struct Position { float x, y, z; };
struct Velocity { float dx, dy, dz; };
struct Health { int current, max; };

int main() {
    World world;
    
    // Create entities
    for (int i = 0; i < 1000; ++i) {
        Entity e = world.CreateEntity();
        world.Add(e, Position{ 0.0f, 0.0f, 0.0f });
        world.Add(e, Velocity{ 1.0f, 0.0f, 0.0f });
        world.Add(e, Health{ 100, 100 });
    }
    
    // Game loop
    float dt = 0.016f; // 60 FPS
    for (int frame = 0; frame < 1000; ++frame) {
        // Movement system (parallel)
        auto moveQuery = world.QueryComponents<Position, Velocity>();
        auto moveParallel = MakeParallel(moveQuery);
        moveParallel.Execute([dt](Position& pos, Velocity& vel) {
            pos.x += vel.dx * dt;
            pos.y += vel.dy * dt;
            pos.z += vel.dz * dt;
        });
        
        // Health system (sequential)
        auto healthQuery = world.QueryComponents<Health>();
        healthQuery.ForEach([&world](Health& health) {
            if (health.current <= 0) {
                // Entity died - handle in separate pass
            }
        });
    }
    
    return 0;
}
```

---

## See Also

- [AthiVegam Architecture](../AthiVegam.md)
- [Jobs System Guide](jobs.md) (coming soon)
- [Performance Profiling](profiling.md) (coming soon)

