# Building AthiVegam

This document describes how to build the AthiVegam game engine on Windows.

## Prerequisites

### Required Software

1. **Visual Studio 2022** (17.5 or later)
   - Workload: "Desktop development with C++"
   - Workload: ".NET desktop development" (for editor, future)
   - Individual component: "C++ CMake tools for Windows"

2. **CMake** (3.28 or later)
   - Download from: https://cmake.org/download/
   - Or install via Visual Studio installer

3. **Git** (2.40 or later)
   - Download from: https://git-scm.com/download/win
   - Required for submodule management

4. **Windows SDK** (10.0.22621.0 or later)
   - Included with Visual Studio 2022
   - Required for D3D12 and platform APIs

### Optional Software

1. **.NET 8 SDK** (for scripting, Phase 6)
   - Download from: https://dotnet.microsoft.com/download/dotnet/8.0

2. **DXC** (DirectX Shader Compiler, for rendering, Phase 4)
   - Included with Windows SDK
   - Or download from: https://github.com/microsoft/DirectXShaderCompiler

## Initial Setup

### 1. Clone the Repository

```bash
git clone https://github.com/srinathupadhyayula/AthiVegam.git
cd AthiVegam
```

### 2. Initialize Submodules (Future)

When external dependencies are added as submodules:

```bash
git submodule update --init --recursive
```

## Building with CMake Presets

### Quick Start

```bash
# Configure for Debug build
cmake --preset windows-debug

# Build
cmake --build build/debug

# Run tests
ctest --preset windows-debug
```

### Available Presets

- **windows-debug**: Debug build with full debugging information
- **windows-release**: Optimized release build
- **windows-relwithdebinfo**: Optimized build with debug info (for profiling)
- **windows-asan**: Debug build with AddressSanitizer enabled

### Build Commands

```bash
# Configure
cmake --preset <preset-name>

# Build
cmake --build build/<preset-name> --config <Debug|Release|RelWithDebInfo>

# Build specific target
cmake --build build/<preset-name> --target AthiVegam_Core

# Clean build
cmake --build build/<preset-name> --target clean

# Run tests
ctest --preset <preset-name>
```

## Building with Visual Studio

### Option 1: Open Folder

1. Open Visual Studio 2022
2. File → Open → Folder
3. Select the AthiVegam root directory
4. Visual Studio will automatically detect CMakeLists.txt
5. Select a CMake preset from the configuration dropdown
6. Build → Build All

### Option 2: Generate Solution

```bash
cmake -S . -B build/vs2022 -G "Visual Studio 17 2022" -A x64
```

Then open `build/vs2022/AthiVegam.sln` in Visual Studio.

## Build Options

Configure build options by editing CMakePresets.json or passing `-D` flags:

```bash
cmake --preset windows-debug -DATHIVEGAM_BUILD_TESTS=OFF
```

### Available Options

- `ATHIVEGAM_BUILD_TESTS` (ON/OFF): Build unit tests (default: ON)
- `ATHIVEGAM_BUILD_EXAMPLES` (ON/OFF): Build examples (default: ON)
- `ATHIVEGAM_BUILD_DOCS` (ON/OFF): Build documentation (default: OFF)
- `ATHIVEGAM_ENABLE_ASAN` (ON/OFF): Enable AddressSanitizer (default: OFF)
- `ATHIVEGAM_ENABLE_UBSAN` (ON/OFF): Enable UndefinedBehaviorSanitizer (default: OFF, GCC/Clang only)

## Running Tests

```bash
# Run all tests
ctest --preset windows-debug

# Run tests with output
ctest --preset windows-debug --output-on-failure

# Run specific test
ctest --preset windows-debug -R TestFrameArena

# Run tests in parallel
ctest --preset windows-debug -j 8
```

## Build Artifacts

Build artifacts are organized as follows:

```
build/<preset-name>/
├── bin/           # Executables and DLLs
├── lib/           # Static libraries
└── tests/         # Test executables
```

## Troubleshooting

### CMake Configuration Fails

**Problem**: CMake cannot find Visual Studio or C++ compiler

**Solution**:
- Ensure Visual Studio 2022 with C++ workload is installed
- Run CMake from "Developer Command Prompt for VS 2022"
- Or specify generator explicitly: `-G "Visual Studio 17 2022"`

### Missing Dependencies

**Problem**: spdlog, GoogleTest, or other dependencies not found

**Solution**:
- Dependencies are automatically fetched via CMake FetchContent
- Ensure you have internet connection during first configure
- Check `build/<preset>/\_deps/` for downloaded dependencies

### C++23 Features Not Available

**Problem**: Compiler errors about C++23 features

**Solution**:
- Update Visual Studio to 17.5 or later
- Ensure `/std:c++latest` flag is set (automatic with presets)
- Check compiler version: `cl.exe` should be 19.35 or later

### Tests Fail to Run

**Problem**: Test executables crash or fail to start

**Solution**:
- Ensure all DLL dependencies are in PATH or same directory as executable
- Run from build directory: `cd build/debug && ctest`
- Check for missing runtime dependencies (Visual C++ Redistributable)

## Performance Builds

For maximum performance:

```bash
# Release build with LTO
cmake --preset windows-release -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
cmake --build build/release --config Release
```

## Continuous Integration

The project uses GitHub Actions for CI. See `.github/workflows/` for CI configuration.

Local CI simulation:

```bash
# Build all configurations
cmake --preset windows-debug && cmake --build build/debug
cmake --preset windows-release && cmake --build build/release

# Run all tests
ctest --preset windows-debug
```

## Next Steps

After successful build:

1. Read [CONTRIBUTING.md](CONTRIBUTING.md) for development workflow
2. Read [TESTING.md](TESTING.md) for testing guidelines
3. See [docs/guides/](docs/guides/) for implementation guides
4. Check [PHASE0_PLAN.md](PHASE0_PLAN.md) for current development phase

## Getting Help

- Check [GitHub Issues](https://github.com/srinathupadhyayula/AthiVegam/issues)
- Read [Documentation](Documentation/)
- Contact: srinathupadhyayula@gmail.com
