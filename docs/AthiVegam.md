# AthiVegam - Game Engine Design Document

## Overview and goals

- **Mission:** A modular, data-oriented game engine in C++23/26 with C#/.NET 8 and Lua 5.4 scripting, targeting Windows with D3D12 primary and Vulkan secondary via Diligent Engine RHI, optimized for indie-scale projects.
- **Design pillars:**
  - **Modularity:** subsystems are swappable behind thin facades.
  - **Data-oriented ECS:** archetype+chunk storage with cache-friendly iteration.
  - **Independent job system:** used by ECS and all subsystems; no hard coupling.
  - **Unified communication:** one messaging substrate powering logging, events, networking, and editor IPC.
  - **Out-of-process editor:** Avalonia UI communicates over protobuf IPC; runtime hosts ImGui overlays.
  - **Hybrid asset pipeline:** runtime import for authoring; offline cooking to deterministic, VCS-friendly packages.

## Architecture map and module boundaries

```
+-------------------------------------------+
| Editor (Avalonia/.NET)                    |
| - Panels, property grids, asset browser   |
| - Play/pause/step                         |
| <-> IPC (Named pipes + Protobuf)          |
+-------------------------------------------+
                | Commands/Streams
                v
+-------------------------------------------+
| Engine Runtime (C++ Core)                 |
|                                           |
| +---------+  +-------------------------+  |
| | Jobs    |  | Communication Layer     |  |
| |         |  | - Logging/Events        |  |
| |         |  | - Networking (ENet)     |  |
| +---------+  | - Editor IPC adapters   |  |
|               +-------------------------+ |
| +---------+  +-------------------------+  |
| | ECS     |  | Scripting               |  |
| |         |  | - CoreCLR/.NET 8 host   |  |
| |         |  | - Lua 5.4 (shared VM)   |  |
| +---------+  +-------------------------+  |
| +---------+  +-------------------------+  |
| | Rendering|  | Physics (Bullet/Box2D) |  |
| | (Diligent)| | + Thin abstraction     |  |
| +---------+  +-------------------------+  |
| +-------------------------------------+   |
| | Asset pipeline & shader toolchain   |   |
| +-------------------------------------+   |
+-------------------------------------------+
```

- **Modules:**
  - **Core:** platform, time, filesystem, threading, memory.
  - **Jobs:** scheduler, fibers, barriers, parallel_for.
  - **Comm:** bus, channels, payload registry, sinks/adapters (logging, IPC, networking).
  - **ECS:** entities, archetypes, chunk storage, queries, systems.
  - **Scripting:** CoreCLR host, C# façade, Lua VM and bridges.
  - **Rendering:** RHI facade over Diligent, frame graph, materials, visibility.
  - **Physics:** abstraction, components, integration passes, event synthesis.
  - **Networking:** ENet transport, ECS components/systems, serialization.
  - **Assets:** import/cook, packages, manifests, shader compile/reflect.
  - **Editor IPC:** protobuf schemas, named-pipe transport, protocol handlers.

## Core runtime (jobs, memory, configuration)

### Job system

- **Model:**
  - **Scheduler:** per-thread work-stealing deques; lock-free MPSC submission.
  - **Waiting:** fibers for blocking tasks; continuations to avoid OS threads stalling.
  - **Hazards:** jobs declare read/write sets (component types, resource IDs); scheduler avoids conflicts.

- **API (C++):**

```cpp
struct JobDesc {
    const char* name;
    uint32_t priority;
    Span<const ResourceKey> reads;
    Span<const ResourceKey> writes;
    Affinity affinity; // CPU core/NUMA/GPU queue hints
};

JobHandle submit(JobDesc desc, Function<void()> fn);
void wait(JobHandle h);
template<typename Fn>
void parallel_for(size_t begin, size_t end, size_t grain, Fn fn);
```

- **API (C# façade):**

```c#
public static class Jobs {
    public static JobHandle Submit(JobDesc desc, Action fn);
    public static void Wait(JobHandle h);
    public static void ParallelFor(long begin, long end, long grain, Action<long> fn);
}
```

- **Phases:** **Label:** Input → Simulation → Physics → Animation → Rendering → Post.

### Memory strategy

- **Allocators:**
  - **Frame arena:** bump reset per frame for transient data.
  - **Chunk allocators:** per-archetype SoA columns; minimize fragmentation.
  - **Pool/slab:** stable handles, small objects (events, command buffers).
  - **Streaming heaps:** double-buffered for GPU uploads and large assets.

- **Handles:**
  - **Stable:** 64-bit with versioning; no raw pointers across module boundaries.
  - **Views:** bounds-checked in debug; fast unchecked in release.

### Configuration and capability flags

- **Graphics:** **Label:** Bindless, mesh shaders, descriptor indexing, dynamic rendering.
- **Physics:** **Label:** CCD, character controllers, multithreaded solver.
- **Networking:** **Label:** Reliable/unreliable channels, compression, encryption (future).
- **Editor IPC:** **Label:** Schema version, protocol version, transport timeouts.

## ECS and composition

> **📖 See [ECS Usage Guide](guides/ecs.md) for detailed documentation and examples.**

### Core ECS specification (Phase 3 - ✅ COMPLETE)

**Status:** Production-ready with 78 comprehensive tests (55 unit + 11 integration + 17 performance)

**Features Implemented:**
- **Entities:** 64-bit ID (32-bit index + 32-bit version); free-list reuse with version validation; `World::Clear()` and `World::GetEntityInfo()` APIs.
- **Archetypes:** Unique component signature key; automatic chunk allocation per archetype; efficient migration on component add/remove.
- **Chunks:** 64 KB; SoA (Structure of Arrays) layout; 64-byte SIMD alignment; capacity calculated per component layout.
- **Components:** Concept-based validation (`Component<T>`); automatic registration; type-safe access via `std::expected<T, Error>`.
- **Queries:** Include/exclude sets; archetype matching; chunk-level iteration; parallel execution via Jobs system integration.
- **Error Handling:** `std::expected` for all operations; comprehensive error codes (InvalidEntity, ComponentNotFound, ArchetypeMismatch, etc.).
- **Thread Safety:** Parallel queries (read-only) are safe; mutations require external synchronization or single-threaded access.

**Performance Characteristics:**
- Entity creation: 5.17M ops/sec (sequential), 18.21M ops/sec (free-list reuse)
- Component access: 13.49M ops/sec (Get), 3.67M ops/sec (Has)
- Query iteration: 203M entities/sec (sequential), 285M entities/sec (parallel)
- Archetype migration: 222K ops/sec (add), 248K ops/sec (remove)
- Memory overhead: ~12 bytes per entity
- Parallel speedup: 1.32x (100K entities), scales better with larger datasets

### System execution

- **Parallel Iteration:** **Label:** Integrated with Jobs system; chunk-level work distribution; automatic thread safety.
- **Query API:** **Label:** Type-safe component access; ForEach and chunk-level iteration; exclude filters.
- **Example (C++):**

```cpp
// Sequential iteration
auto query = world.QueryComponents<Transform, Velocity>();
query.ForEach([](Transform& t, Velocity& v) {
    t.x += v.dx;
    t.y += v.dy;
    t.z += v.dz;
});

// Parallel iteration (integrated with Jobs system)
auto parallel = MakeParallel(query);
parallel.Execute([](Transform& t, Velocity& v) {
    t.x += v.dx;
    t.y += v.dy;
    t.z += v.dz;
});

// Chunk-level parallel iteration (SIMD-friendly)
parallel.ExecuteChunks([](Transform* transforms, Velocity* velocities, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        transforms[i].x += velocities[i].dx;
    }
});
```

### Hybrid composition layer

- **Game objects:** **Label:** lightweight handles mapping to ECS entities; script components bind behavior.
- **Prefabs:** **Label:** declarative component sets and overrides; instantiate to ECS; hierarchy stored in TransformGraph.
- **Scene graph:** **Label:** parent-child relation maintained outside ECS structural data; world matrices solved post-edit.

### Reflection and serialization

- **Source generators:** **Label:** C++ metadata emission (fields, names, ranges); C# attributes for managed components.
- **Binary IO:** **Label:** custom POD format; versioned; diff-friendly manifests.

## Communication layer and IPC

### Bus and channels

- **Topology:** **Label:** global bus; typed channels keyed by topic (string or hashed ID).
- **Payloads:** **Label:** internal custom POD; external protobuf at boundaries (IPC/network/file).
- **Queues:** **Label:** lock-free MPSC; optional ring buffer; priorities and QoS; back-pressure flags.
- **Delivery modes:**
  - **Sync:** immediate (game-critical).
  - **Async:** job-backed (default).
  - **Buffered:** frame-scoped drainage.

### Logging and events

- **Logging:** **Label:** severity (Trace/Debug/Info/Warn/Error), domains, sinks (console/file/IPC).
- **Events:** **Label:** sync by default; async for telemetry; event categorization (Gameplay/UI/System).

### Editor IPC defaults

- **Transport:** **Label:** Named pipes (Windows); binary protobuf messages.
- **Timeouts:** **Label:** 5s handshake; 30s idle disconnect; heartbeat every 5s.
- **Reconnect:** **Label:** exponential backoff (1s → 2s → 5s → 10s → 20s → 30s); jitter ±10%.
- **Versioning:** **Label:** schema_version (u16), engine_version (semver string), editor_version; graceful downgrade policy.

### IPC message schemas (protobuf)

```Proto
message Handshake {
  uint16 schema_version = 1;
  string engine_version = 2;
  string editor_version = 3;
}

message Command {
  oneof payload {
    LoadScene load_scene = 1;
    SpawnPrefab spawn_prefab = 2;
    SetProperty set_property = 3;
    Control control = 4; // play/pause/step
  }
}

message Stream {
  oneof payload {
    LogEntry log = 1;
    PerfCounters perf = 2;
    EcsInspect ecs = 3;
    FrameCapture capture = 4;
  }
}
```

- **Routing:** **Label:** editor sends Command; engine returns Ack/Nack; streams are fire-and-forget on dedicated channels.

## Rendering subsystem (RHI facade over Diligent)

### RHI facade

- **Goals:** **Label:** unify resource model and PSO across D3D12/Vulkan; hide backend specifics; expose capabilities.
- **Core interfaces (C++):**

```cpp
struct TextureDesc { uint32_t w,h,mips; Format fmt; Usage usage; };
struct BufferDesc { size_t size; Usage usage; CPUAccess cpu; };
struct PipelineDesc { ShaderSet shaders; RenderState state; Layout layout; };

struct IRhi {
  TextureHandle create_texture(const TextureDesc&);
  BufferHandle  create_buffer(const BufferDesc&);
  PipelineHandle create_pipeline(const PipelineDesc&);
  void submit(CommandList&&);
  Capabilities caps() const;
};
```

### Frame graph

- **Passes:** **Label:** declarative nodes with read/write resources; automatic barrier inference; transient aliasing.
- **Submission:** **Label:** multi-threaded command recording; device queues mapped by backend; timeline semaphores for Vulkan.
- **Visibility:** **Label:** ECS → culling → draw packet building per archetype (Transform, Renderable, Material).

### Materials and shaders

- **Authoring:** **Label:** HLSL only; DXC compiles to DXIL (D3D12) and SPIR-V (Vulkan).
- **Reflection metadata:**

- **Bindings:** textures, samplers, buffers (slots/sets).
- **Push/root constants:** sizes and offsets.
- **Descriptor sets/layouts:** per stage.
- **Vertex I/O:** attributes and semantics; render target formats.
- **Specialization constants:** keys and defaults.
- **Entry points:** VS/PS/CS/MS/AS names.
- **Debug names:** for editor display.

### 2D/3D features

- **2D:** **Label:** batched sprites/tiles; atlases; instanced quads; grid utilities.
- **3D:** **Label:** PBR forward+/clustered; GPU-driven indirect; frustum/occlusion culling.
- **Modern GPU:** **Label:** bindless, descriptor indexing, mesh shaders gated by caps; ray tracing postponed.

## Physics and networking subsystems

### Physics abstraction

- **Backends:** **Label:** Bullet (3D), Box2D (2D).
- **ECS components:**
  - **RigidBody:** mass, inertia, velocities, damping.
  - **Collider:** shape type, dims, offset, material.
  - **Joint:** type, anchors, limits, motors.
  - **PhysicsMaterial:** friction, restitution, density.
  - **LayerMask:** groups and filters.
- **Service interface (C++):**

```cpp
struct IPhysics {
  PhysicsWorldHandle create_world(const PhysicsSettings&);
  BodyHandle create_body(PhysicsWorldHandle, const RigidBody&, const Collider&);
  void step(PhysicsWorldHandle, float dt);
  bool raycast(...); bool overlap(...);
}
```

- **Events:** **Label:** ContactBegin/End, TriggerEnter/Exit, Sleep/Wake, Stay.
  - **Stay synthesis:** manifold tracking across fixed steps; emits Stay while contact persists.

## Networking (ENet transport)

- **Transport:** **Label:** reliable/unreliable UDP; channels; sequencing.
- **ECS components:**
  - **NetEndpoint:** address, port, protocol, role.
  - **NetConnection:** state, remote ID, channels.
  - **NetChannel:** reliability, ordering, priority, compression.
  - **Replicated:** opt-in marker; policy.
  - **AuthorityTag:** ownership hints.
- **Systems:**
  - **Connect/Accept:** establish endpoints.
  - **Send/Receive:** interface with Comm Layer channels.
  - **Serialize/Deserialize:** internal POD for in-process; protobuf at external boundaries.
  - **Replication defaults:** full sync on join; per-frame deltas; float quantization at 1/1000; entity map remap.

## Scripting integration (C# and Lua)

### C#/.NET 8 host

- **Hosting:** **Label:** HostFXR embedding, isolated domains for editor/runtime; hot-reload via shadow assemblies.
- **Interop:** **Label:** extern "C" ABI with blittable structs; 64-bit opaque handles; Span/ReadOnlySpan for buffers.
- **Managed façade:**

```cpp
public readonly struct Entity {
  public readonly ulong Id;
  public T Get<T>() where T: unmanaged;
  public void Set<T>(T data) where T: unmanaged;
  public void Add<T>(T data) where T: unmanaged;
}

public abstract class ScriptComponent {
  public Entity Entity { get; internal set; }
  public virtual void Update(float dt) {}
  public virtual void FixedUpdate(float dt) {}
}
```

- **Jobs API:** **Label:** mirrors native; validated read/write sets; no exception unwinding into native.

### Lua 5.4 shared VM

- **Bridge:** **Label:** C++ via Lua C API; C# via KeraLua/NLua; single VM per process.
- **Namespace:** **Label:** shared engine tables/functions; consistent registration across languages.
- **Interaction:** **Label:** Comm Layer offers Lua-safe queues to exchange events with C# without tight coupling.

## Asset pipeline and shader toolchain

### Runtime import (authoring)

- **Formats:** **Label:** glTF, PNG, WAV/OGG, etc.; converted in-memory to engine-native blobs.
- **Shaders:** **Label:** compile on demand with DXC; cache binaries; reflect bindings.

### Offline cooking (builds)

- **Package format:**
  - **Header:** type, version, UUID, hash, dependencies.
  - **Chunks:** deterministic order, fixed seeds; Zstd/LZ4 compression.
  - **Relocation table:** offsets to subresources.
  - **VCS-friendly:** JSON sidecar manifest for human diffs; reproducible builds.
- **Dependency graph:** **Label:** hashes for invalidation; incremental cooking; content addressable cache.

### Shader reflection fields for editor

- **Bindings:** **Label:** textures/samplers/buffers with slots/sets.
- **Push/root constants:** **Label:** sizes, offsets.
- **Descriptor sets/layouts:** **Label:** per stage mapping.
- **Vertex I/O:** **Label:** attribute names, types, semantics.
- **Render targets:** **Label:** formats, counts.
- **Specialization constants:** **Label:** identifiers and defaults.
- **Entry points:** **Label:** per stage names.
- **Debug names:** **Label:** for display.

## Testing, configuration, and implementation milestones

### Unit testing priorities

- **ECS:** **Label:** lifecycle, migration invariants, query correctness.
- **Allocators:** **Label:** fragmentation, bounds, stress.
- **Jobs:** **Label:** fairness, hazards, determinism.
- **Comm:** **Label:** routing, back-pressure, payload integrity.
- **Scripting:** **Label:** interop, hot-reload state handoff.
- **Physics:** **Label:** event synthesis, filters, solver stability.
- **Networking:** **Label:** reliability, ordering, reconnect flows.
- **Rendering:** **Label:** pass dependencies, barrier correctness, resource lifetimes.

### Recommended implementation order (milestones)

1. **Foundation:**

	- **Goal:** platform, memory, time, logging basics; unit test harness.
	- **Deliverables:** frame arena, pool/slab, basic Comm Layer with console/file sinks.

2. **Job system:**

	- **Goal:** scheduler, parallel_for, hazards.
	- **Deliverables:** microbenches; ECS can use parallel_for immediately.

3. **ECS MVP:**

	- **Goal:** entities, archetypes, chunks, queries.
	- **Deliverables:** simple movement/render-less simulation; tests for migration and queries.

4. **RHI facade and minimal renderer:**

	- **Goal:** Diligent integration; frame graph; swapchain; sprite/quad rendering.
	- **Deliverables:** 2D renderer for Frogger prototype; material reflection.

5. **Editor IPC + minimal Avalonia app:**

	- **Goal:** named pipes, protobuf schemas, scene/property panels.
	- **Deliverables:** play/pause, property editing; ImGui debug overlay in runtime.

6. **Scripting host:**

	- **Goal:** CoreCLR embedding, C# façade; Lua VM and bridge.
	- **Deliverables:** script-driven behavior components; job scheduling from C#.

7. **Physics abstraction:**

	- **Goal:** Bullet/Box2D integration; ECS components; events including Stay.
	- **Deliverables:** basic collisions; character controller stub; fixed-step loop.

8. **Networking (ENet):**

	- **Goal:** transport init; connect/send/receive; replication defaults.
	- **Deliverables:** simple multiplayer test (entity position sync) using Comm Layer.

9. **Asset pipeline and shader toolchain:**

	- **Goal:** runtime import; offline cooking; DXC compile; reflection metadata.
	- **Deliverables:** package builder; deterministic outputs; editor asset browser.

10. **Polish and profiling:**

	- **Goal:** perf captures, culling, GPU-driven indirect; job tracing; ECS stats.
	- **Deliverables:** dashboards in ImGui/editor streams; optimization passes.

## Acceptance criteria per milestone

- **Foundation:** **Label:** memory tests passing; logging routes to console/file.
- **Jobs:** **Label:** parallel_for scales across cores; hazards prevent conflicts.
- **ECS MVP:** **Label:** migration is stable; queries return correct archetypes/views.
- **Renderer:** **Label:** frame graph validates barriers; sprites render with bindless cache.
- **Editor IPC:** **Label:** roundtrip commands; property edits reflected live.
- **Scripting:** **Label:** C# and Lua can get/set components; job scheduling works from scripts.
- **Physics:** **Label:** contact/trigger events delivered; Stay synthesized correctly.
- **Networking:** **Label:** reliable/unreliable channels verified; reconnection succeeds.
- **Assets/Shaders:** **Label:** cooked packages load; reflection populates editor UI.
- **Polish:** **Label:** perf and ECS stats visible; baseline FPS and latency targets met.

Appendix: key contracts and enums (concise)

```cpp
// Comm Layer payload registration
template<typename T>
ChannelHandle register_channel(const char* topic);

struct LogEntry { Severity sev; const char* domain; const char* msg; };

// ECS component registration
REGISTER_COMPONENT(Transform);
REGISTER_COMPONENT(Velocity);

// RHI Capabilities
struct Capabilities {
  bool bindless;
  bool meshShaders;
  bool descriptorIndexing;
  bool dynamicRendering;
};

// Physics events
enum class PhysicsEventType { ContactBegin, ContactEnd, TriggerEnter, TriggerExit, Sleep, Wake, Stay };

// Networking channel policy
struct NetPolicy { bool reliable; bool ordered; uint8_t priority; bool compress; };
```

🚀 Recommended Implementation Order

Phase 0: Foundation

- **Platform layer:** time, filesystem, threading primitives, memory allocators.
- **Unit test harness:** GoogleTest or Catch2 for C++, xUnit for C#.

Phase 1: Communication Layer (minus networking)

- Build the **bus + channels** with POD payloads, sync/async delivery, logging/event sinks.
- This gives you a unified way to trace and debug everything else as you add it.

Phase 2: Job System

- Implement the work-stealing scheduler, fibers, and hazard tracking.
- ECS and later subsystems will use this for parallelism.

Phase 3: ECS Core

- Entities, archetypes, chunks, queries.
- Integrate with job system for parallel iteration.
- Add reflection/registration macros.

Phase 4: Rendering RHI Facade

- Wrap Diligent Engine, implement frame graph, minimal sprite renderer.
- Use ECS to drive rendering (Transform + Renderable components).

Phase 5: Editor IPC

- Protobuf schemas, named pipes transport, Avalonia shell.
- ImGui overlays in runtime for debug.

Phase 6: Scripting

- CoreCLR host, C# façade, Lua VM integration.
- ECS component access from scripts.

Phase 7: Physics Abstraction

- Bullet + Box2D integration, ECS components, event synthesis.

Phase 8: Networking

- ENet transport, ECS components, replication defaults.

Phase 9: Asset Pipeline

- Runtime import, offline cooking, shader toolchain.

🧩 Skeleton Header Stubs

`CommLayer.hpp`

```cpp
#pragma once
#include <cstdint>
#include <functional>
#include <string_view>

namespace Engine::Comm {

using ChannelId = uint64_t;

struct Payload {
    const void* data;
    size_t size;
};

enum class DeliveryMode { Sync, Async, Buffered };

struct ChannelDesc {
    std::string_view topic;
    DeliveryMode mode;
};

class Channel {
public:
    Channel(ChannelId id, ChannelDesc desc);
    void publish(const Payload& p);
    void subscribe(std::function<void(const Payload&)> handler);
private:
    ChannelId id_;
    ChannelDesc desc_;
};

class Bus {
public:
    ChannelId register_channel(const ChannelDesc& desc);
    Channel& get(ChannelId id);
private:
    // internal storage
};

} // namespace Engine::Comm
```

`JobSystem.hpp`

```cpp
#pragma once
#include <functional>
#include <span>
#include <cstdint>

namespace Engine::Jobs {

using JobHandle = uint64_t;

struct ResourceKey { uint64_t id; };

struct JobDesc {
    const char* name;
    uint32_t priority;
    std::span<const ResourceKey> reads;
    std::span<const ResourceKey> writes;
};

class Scheduler {
public:
    JobHandle submit(const JobDesc& desc, std::function<void()> fn);
    void wait(JobHandle h);

    template<typename Fn>
    void parallel_for(size_t begin, size_t end, size_t grain, Fn fn);
};

} // namespace Engine::Jobs
```

`ECS.hpp`

```cpp
#pragma once
#include <cstdint>
#include <vector>
#include <tuple>

namespace Engine::ECS {

using EntityId = uint64_t;

struct Entity {
    uint32_t index;
    uint32_t version;
};

class World {
public:
    Entity create();
    void destroy(Entity e);

    template<typename T>
    void add(Entity e, const T& component);

    template<typename T>
    T& get(Entity e);

    template<typename... Ts>
    struct Query {
        // iteration API
    };

    template<typename... Ts>
    Query<Ts...> query();
};

} // namespace Engine::ECS
```

🔑 Why this order works

- **Comm Layer first:** gives you logging/events to debug everything else.
- **Job System second:** ensures parallelism is consistent across ECS, rendering, physics.
- **ECS third:** builds on jobs + comms, and becomes the backbone for all subsystems.
- **Rendering next:** you can immediately test ECS by drawing sprites/tiles.
- **Editor IPC:** once ECS+rendering exist, you can inspect and manipulate entities.
- **Scripting:** now you can drive ECS from C#/Lua.
- **Physics/Networking:** plug into ECS and Comm Layer.
- **Assets:** finalize pipeline once core runtime is stable.
