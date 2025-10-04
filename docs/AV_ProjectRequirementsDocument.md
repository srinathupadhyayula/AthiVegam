# 📑 AthiVegam Engine – PRD & Actionable Tasklist

## 1. Mission

Build a modular, data‑oriented game engine in **C++23/26** with **C#/.NET 8** and **Lua 5.4** scripting, targeting **Windows** with **D3D12 (primary)** and **Vulkan (secondary)** via **Diligent Engine RHI**, optimized for **indie‑scale projects**.

## 2. High‑Level Milestones (Implementation Order)

1. **Foundation**
    
    - Platform layer (time, filesystem, threading, memory allocators).
        
    - Unit test harness (GoogleTest/Catch2 for C++, xUnit for C#).
        
    - Deliverables: frame arena, pool/slab allocators, logging basics.
        
2. **Communication Layer (minus networking)**
    
    - Bus + channels with POD payloads.
        
    - Sync/async delivery modes.
        
    - Logging/events subsystem.
        
    - Deliverables: console/file sinks, event routing.
        
3. **Job System**
    
    - Work‑stealing scheduler, fibers, hazard tracking.
        
    - Deliverables: `submit`, `wait`, `parallel_for`.
        
    - Acceptance: parallel_for scales across cores, hazard enforcement.
        
4. **ECS Core**
    
    - Entities, archetypes, chunks, queries.
        
    - Hybrid composition layer (prefabs, scene graph).
        
    - Deliverables: simple movement simulation, query tests.
        
5. **Rendering RHI Facade**
    
    - Wrap Diligent Engine.
        
    - Frame graph, swapchain, sprite/quad renderer.
        
    - Deliverables: 2D renderer for Frogger prototype.
        
6. **Editor IPC**
    
    - Protobuf schemas, named pipes transport.
        
    - Avalonia shell (panels, property grids).
        
    - Deliverables: play/pause, property editing, ImGui debug overlay.
        
7. **Scripting**
    
    - CoreCLR host, C# façade.
        
    - Lua 5.4 VM bridged via NLua/KeraLua.
        
    - Deliverables: script‑driven components, job scheduling from C#.
        
8. **Physics Abstraction**
    
    - Bullet (3D) + Box2D (2D).
        
    - ECS components (RigidBody, Collider, Joint).
        
    - Deliverables: collisions, Stay events, character controller stub.
        
9. **Networking (ENet)**
    
    - Reliable/unreliable UDP transport.
        
    - ECS components (NetEndpoint, NetConnection, Replicated).
        
    - Deliverables: multiplayer test syncing entity positions.
        
10. **Asset Pipeline & Shader Toolchain**
    
    - Runtime import (glTF, PNG, WAV).
        
    - Offline cooking (binary packages, JSON manifests).
        
    - Shader compilation via DXC → DXIL/SPIR‑V.
        
    - Deliverables: package builder, editor asset browser.
        
11. **Polish & Profiling**
    
    - Perf captures, ECS stats, GPU‑driven indirect.
        
    - Deliverables: dashboards in ImGui/editor streams.
        

## 3. Detailed Tasklist (LLM‑Friendly)

### Phase 0 – Foundation ✅ COMPLETE

- [x] Implement `FrameArena` allocator.

- [x] Implement `PoolAllocator` for small objects.

- [x] Implement `SlabAllocator` for stable handles.

- [x] Add `Time` module (high‑res timers, delta time).

- [x] Add `Filesystem` abstraction (read/write, path utils).

- [x] Add `Logger` with console/file sinks.

- [x] Setup GoogleTest harness.

- [x] Implement `Application` base class with lifecycle hooks.

- [x] Implement `EntryPoint.hpp` with main() function.

- [x] Create comprehensive test suite (EngineTest).

- [x] Resolve Windows API naming conflicts (DeleteFile → RemoveFile).

- [x] Configure CLion for one-click run/debug workflow.
    

### Phase 1 – Communication Layer ✅ COMPLETE

- [x] Define `ChannelDesc`, `Channel`, `Bus`.

- [x] Implement `publish`/`subscribe`.

- [x] Add delivery modes: Sync, Async (stubbed), Buffered.

- [x] Add `LogEntry` struct and severity levels.

- [x] Add event categories (Gameplay/UI/System).

- [x] Unit tests: publish/subscribe, sync vs async.
    

### Phase 2 – Job System

- [ ] Implement `Scheduler` with worker threads.
    
- [ ] Implement `submit`, `wait`, `parallel_for`.
    
- [ ] Add hazard tracking (read/write sets).
    
- [ ] Add fiber support for blocking tasks.
    
- [ ] Unit tests: parallel_for scaling, hazard enforcement.
    

### Phase 3 – ECS Core ✅ COMPLETE

- [x] Define `Entity` (index+version).

- [x] Implement free‑list reuse.

- [x] Implement archetypes + chunk storage (64KB).

- [x] Implement queries with include/exclude sets.

- [x] Implement parallel iteration with Jobs system integration.

- [x] Add World::Clear() and World::GetEntityInfo() APIs.

- [x] Comprehensive error handling with std::expected.

- [x] Unit tests: lifecycle, migration, query correctness (55 tests).

- [x] Integration tests: large-scale validation, stress tests (11 tests).

- [x] Performance benchmarks: throughput and scaling analysis (17 tests).

- [x] Documentation: usage guide and performance characteristics.

- [ ] Add prefab system (component sets + overrides) - DEFERRED to Phase 4+.

- [ ] Add scene graph (TransformGraph) - DEFERRED to Phase 4+.
    

### Phase 4 – Rendering

- [ ] Wrap Diligent Engine into `IRhi` facade.
    
- [ ] Implement `TextureDesc`, `BufferDesc`, `PipelineDesc`.
    
- [ ] Implement frame graph with automatic barriers.
    
- [ ] Implement sprite/quad renderer.
    
- [ ] Unit tests: frame graph validation, resource lifetimes.
    

### Phase 5 – Editor IPC

- [ ] Define protobuf schemas (Handshake, Command, Stream).
    
- [ ] Implement named pipes transport.
    
- [ ] Implement Avalonia shell with property grid.
    
- [ ] Implement ImGui debug overlay in runtime.
    
- [ ] Unit tests: roundtrip commands, property edits.
    

### Phase 6 – Scripting

- [ ] Embed CoreCLR via HostFXR.
    
- [ ] Implement managed façade (`Entity`, `ScriptComponent`).
    
- [ ] Integrate Lua 5.4 VM with NLua/KeraLua.
    
- [ ] Add Lua‑C# bridge via Comm Layer.
    
- [ ] Unit tests: C#/Lua can get/set components, hot‑reload.
    

### Phase 7 – Physics

- [ ] Wrap Bullet (3D) and Box2D (2D).
    
- [ ] Define ECS components (RigidBody, Collider, Joint).
    
- [ ] Implement Stay event synthesis.
    
- [ ] Unit tests: collisions, triggers, Stay events.
    

### Phase 8 – Networking

- [ ] Integrate ENet transport.
    
- [ ] Define ECS components (NetEndpoint, NetConnection, Replicated).
    
- [ ] Implement replication defaults (full sync, deltas, quantization).
    
- [ ] Unit tests: reliable/unreliable channels, reconnect flows.
    

### Phase 9 – Assets/Shaders

- [ ] Implement runtime importers (glTF, PNG, WAV).
    
- [ ] Implement offline cooking pipeline.
    
- [ ] Implement package format (binary + JSON manifest).
    
- [ ] Integrate DXC shader compilation.
    
- [ ] Extract reflection metadata (bindings, I/O, constants).
    
- [ ] Unit tests: cooked packages load, reflection populates editor UI.
    

## 4. Acceptance Criteria (LLM‑Friendly)

- **Comm Layer:** publish/subscribe works, logs route to console/file.
    
- **Jobs:** parallel_for scales across cores, hazards prevent conflicts.
    
- **ECS:** entity lifecycle stable, queries correct.
    
- **Rendering:** sprites render, frame graph validates barriers.
    
- **Editor IPC:** roundtrip commands succeed, property edits reflected live.
    
- **Scripting:** C#/Lua can manipulate ECS, jobs schedule from scripts.
    
- **Physics:** collisions and Stay events delivered.
    
- **Networking:** ENet reliable/unreliable verified, reconnection works.
    
- **Assets:** cooked packages load, reflection metadata visible in editor.