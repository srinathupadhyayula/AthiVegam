# CLion Setup Guide for AthiVegam

This guide will help you configure CLion to build and run the AthiVegam game engine.

## Prerequisites

- **CLion 2024.1+** (or any recent version)
- **Visual Studio 2022** with C++ development tools
- **CMake 3.28+** (bundled with CLion)
- **Ninja build system** (bundled with CLion)

## Step 1: Open the Project

1. Launch CLion
2. Click **File → Open**
3. Navigate to `D:/Projects/Active/C++/AthiVegam`
4. Click **OK**

CLion will automatically detect the CMake project and start loading it.

## Step 2: Configure CMake Toolchain

1. Go to **File → Settings** (or **Ctrl+Alt+S**)
2. Navigate to **Build, Execution, Deployment → Toolchains**
3. Ensure **Visual Studio** toolchain is configured:
   - **Name**: Visual Studio
   - **Architecture**: amd64
   - **CMake**: Bundled
   - **Build Tool**: Ninja (bundled)
   - **C Compiler**: Detected from Visual Studio
   - **C++ Compiler**: Detected from Visual Studio

## Step 3: Configure CMake Profiles

1. Go to **File → Settings → Build, Execution, Deployment → CMake**
2. You should see the following profiles (auto-detected from `CMakePresets.json`):

### Debug Profile
- **Name**: windows-debug
- **Build type**: Debug
- **Toolchain**: Visual Studio
- **CMake options**: (auto-configured from preset)
- **Build directory**: `build/windows-debug`
- **Build options**: `-j 16` (adjust based on your CPU cores)

### Release Profile
- **Name**: windows-release
- **Build type**: Release
- **Toolchain**: Visual Studio
- **Build directory**: `build/windows-release`

### RelWithDebInfo Profile
- **Name**: windows-relwithdebinfo
- **Build type**: RelWithDebInfo
- **Toolchain**: Visual Studio
- **Build directory**: `build/windows-relwithdebinfo`

3. Click **OK** to save

## Step 4: Initial CMake Configuration

1. CLion will automatically run CMake configuration
2. Wait for the process to complete (check the bottom status bar)
3. If you see any errors, click **File → Reload CMake Project**

## Step 5: Build the Project

### Option A: Build All
1. Click **Build → Build Project** (or **Ctrl+F9**)
2. Wait for compilation to complete

### Option B: Build Specific Target
1. Click the **Build Configuration** dropdown (top-right, near the Run button)
2. Select a target:
   - **AthiVegam_Core** - Core library only
   - **CoreTest** - Core test executable
   - **ALL_BUILD** - Everything
3. Click the **Build** button (hammer icon)

## Step 6: Run the Core Test

1. Click the **Run Configuration** dropdown (top-right)
2. Select **CoreTest**
3. Click the **Run** button (green play icon) or press **Shift+F10**

You should see output like:
```
=== AthiVegam Core Test ===

[Platform]
  Logical Cores: 32
  Physical Cores: 16
  Page Size: 4096 bytes
  Cache Line Size: 64 bytes

[Time]
  Current Time: 113113 seconds

[Threading]
  Thread ID: 36812

[Memory]
  Allocated 1024 bytes (64-byte aligned)
  Address: 0000020000010000
  Is Aligned: Yes
  Freed successfully

=== All Tests Passed ===
```

## Step 7: Debug Configuration

To debug the CoreTest executable:

1. Set breakpoints by clicking in the gutter (left of line numbers)
2. Click the **Debug** button (bug icon) or press **Shift+F9**
3. Use the debug toolbar to:
   - **Step Over** (F8)
   - **Step Into** (F7)
   - **Step Out** (Shift+F8)
   - **Resume** (F9)

## Troubleshooting

### Issue: "CMake Error: Could not find CMAKE_C_COMPILER"

**Solution**: Ensure Visual Studio 2022 is properly installed with C++ development tools.

1. Open **Visual Studio Installer**
2. Click **Modify** on Visual Studio 2022
3. Ensure **Desktop development with C++** workload is installed
4. Restart CLion

### Issue: "Ninja: command not found"

**Solution**: Use the bundled Ninja from CLion.

1. Go to **Settings → Build, Execution, Deployment → Toolchains**
2. Set **Build Tool** to **Bundled Ninja**

### Issue: Build fails with "SetThreadDescription not declared"

**Solution**: This has been fixed. The code now dynamically loads `SetThreadDescription` at runtime.

If you still see this error, ensure you're building from the latest code.

### Issue: Warnings about "stdext::checked_array_iterator"

**Solution**: These are harmless warnings from spdlog. They don't affect functionality.

To suppress them, add to `CMakeLists.txt`:
```cmake
target_compile_definitions(AthiVegam_Core PRIVATE
    _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
)
```

## Project Structure in CLion

```
AthiVegam/
├── AthiVegam/
│   └── Core/
│       ├── Memory/          # Memory management
│       ├── Platform/        # Platform abstraction
│       ├── Types.hpp        # Core types
│       └── Result.hpp       # Error handling
├── examples/
│   └── 00_CoreTest/         # Core functionality test
├── tests/                   # Unit tests (GoogleTest)
├── docs/                    # Documentation
├── CMakeLists.txt           # Root CMake file
└── CMakePresets.json        # CMake presets for CLion
```

## Keyboard Shortcuts

- **Ctrl+F9**: Build project
- **Shift+F10**: Run
- **Shift+F9**: Debug
- **Ctrl+F2**: Stop
- **Alt+Shift+F10**: Select run configuration
- **Alt+Shift+F9**: Select debug configuration
- **Ctrl+Shift+F10**: Run context configuration (run current file)

## Next Steps

1. **Write Tests**: Add unit tests in the `tests/` directory
2. **Add Features**: Implement additional engine modules
3. **Profile Performance**: Use CLion's built-in profiler
4. **Code Analysis**: Enable CLion's code inspections for better code quality

## Additional Resources

- [CLion Documentation](https://www.jetbrains.com/help/clion/)
- [CMake Documentation](https://cmake.org/documentation/)
- [AthiVegam Project Documentation](./README.md)
