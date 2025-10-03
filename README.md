# AthiVegam Game Engine

A modular, data-oriented game engine in C++23/26 with C#/.NET 8 and Lua 5.4 scripting, targeting Windows with D3D12 (primary) and Vulkan (secondary) via Diligent Engine RHI, optimized for indie-scale projects.

## 🎯 Project Status

**Current Phase**: Phase 0 - Foundation (Initial Setup Complete)

The project structure, build system, and memory bank have been initialized. Ready to begin Phase 0 implementation.

## 🚀 Quick Start

### Prerequisites

- Visual Studio 2022 (17.5+) with C++ workload
- CMake 3.28+
- Windows SDK 10.0.22621.0+
- Git 2.40+

### Build

```bash
# Clone repository
git clone https://github.com/srinathupadhyayula/AthiVegam.git
cd AthiVegam

# Configure and build
cmake --preset windows-debug
cmake --build build/debug

# Run tests
ctest --preset windows-debug
```

See [BUILD.md](BUILD.md) for detailed build instructions.

## 📋 Design Pillars

1. **Modularity**: Swappable subsystems behind thin facades
2. **Data-Oriented ECS**: Archetype + chunk storage for cache-friendly iteration
3. **Independent Job System**: Work-stealing scheduler with hazard tracking
4. **Unified Communication**: Single messaging substrate for logging, events, IPC, networking
5. **Out-of-Process Editor**: Avalonia UI with protobuf IPC
6. **Hybrid Asset Pipeline**: Runtime import + offline cooking

## 🏗️ Architecture

```
Engine Runtime (C++)
├── Core (Platform, Memory, Time, Filesystem)
├── Jobs (Work-stealing scheduler, parallel_for)
├── Comm (Message bus, logging, events)
├── ECS (Entities, archetypes, chunks, queries)
├── Rendering (Diligent Engine RHI, frame graph)
├── Physics (Bullet 3D, Box2D 2D)
├── Scripting (CoreCLR/.NET 8, Lua 5.4)
├── Networking (ENet transport, replication)
├── Assets (Import, cooking, shader compilation)
└── EditorIPC (Protobuf, named pipes)

Editor (C#/Avalonia)
└── Parugu (Property grids, scene hierarchy, asset browser)
```

## 📚 Documentation

- **[BUILD.md](BUILD.md)** - Build instructions and prerequisites
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Development workflow and guidelines
- **[Documentation/](Documentation/)** - Design documents and requirements
- **Memory Bank** - Persistent knowledge base (see allpepper memory bank)
  - [Project Overview](memory-bank:project-overview.md)
  - [Architecture](memory-bank:architecture.md)
  - [Technology Stack](memory-bank:tech-stack.md)
  - [Requirements](memory-bank:requirements.md)
  - [Development Standards](memory-bank:development-standards.md)
  - [Implementation Roadmap](memory-bank:implementation-roadmap.md)

## 🗺️ Implementation Roadmap

| Phase | Status | Description |
|-------|--------|-------------|
| 0: Foundation | 🔄 In Progress | Platform, memory, logging |
| 1: Communication | ⏳ Planned | Message bus, events |
| 2: Job System | ⏳ Planned | Work-stealing scheduler |
| 3: ECS Core | ⏳ Planned | Entities, archetypes, queries |
| 4: Rendering | ⏳ Planned | Diligent Engine, 2D sprites |
| 5: Editor IPC | ⏳ Planned | Avalonia editor, ImGui |
| 6: Scripting | ⏳ Planned | C# and Lua integration |
| 7: Physics | ⏳ Planned | Bullet/Box2D |
| 8: Networking | ⏳ Planned | ENet replication |
| 9: Assets | ⏳ Planned | Import, cooking, shaders |
| 10: Integration | ⏳ Planned | Frogger prototype |
| 11: Polish | ⏳ Planned | Optimization, profiling |

**Estimated Duration**: 38 weeks (~9 months)

## 🛠️ Technology Stack

### Core
- **Language**: C++23/26
- **Build System**: CMake 3.28+
- **Testing**: GoogleTest
- **Logging**: spdlog
- **Memory**: mimalloc

### Graphics
- **RHI**: Diligent Engine (D3D12/Vulkan)
- **Shaders**: DXC (HLSL → DXIL/SPIR-V)
- **Debug UI**: ImGui

### Scripting
- **C#**: .NET 8 (CoreCLR hosting)
- **Lua**: Lua 5.4

### Physics
- **3D**: Bullet Physics 3
- **2D**: Box2D

### Networking
- **Transport**: ENet (reliable/unreliable UDP)

### Editor
- **UI Framework**: Avalonia 11.0+
- **IPC**: Protocol Buffers + Named Pipes

## 🎮 Target Use Cases

- 2D games (platformers, puzzle games, roguelikes)
- Small 3D games (first-person experiences)
- Rapid prototyping and game jams
- Educational tool for engine architecture

## 📊 Performance Targets

- **Frame Rate**: 60 FPS with 10,000 entities
- **Memory**: < 1MB allocations per frame
- **Job Scaling**: Linear scaling up to 8 cores
- **Cache Efficiency**: > 80% L1/L2 hit rate

## 🤝 Contributing

We welcome contributions! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Development Setup

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes and add tests
4. Commit your changes (`git commit -m '[Module] Add amazing feature'`)
5. Push to the branch (`git push origin feature/amazing-feature`)
6. Open a Pull Request

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👤 Author

**Srinath Upadhyayula**
- GitHub: [@srinathupadhyayula](https://github.com/srinathupadhyayula)
- Email: srinathupadhyayula@gmail.com

## 🙏 Acknowledgments

- Diligent Engine for cross-platform graphics abstraction
- Unity DOTS for ECS architecture inspiration
- Molecular Matters for job system design patterns
- The game engine development community

## 📖 Additional Resources

- [Design Document](Documentation/AthiVegam.md) - Comprehensive engine design
- [Requirements Document](Documentation/AV_ProjectRequirementsDocument.md) - Detailed requirements
- [Phase 0 Plan](PHASE0_PLAN.md) - Current development phase details

---

**Note**: This project is in active development. The API and architecture may change as we progress through implementation phases.
