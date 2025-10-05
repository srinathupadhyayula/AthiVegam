# Core Module Unit Tests

## Overview

Comprehensive unit tests for the AthiVegam Engine Core module, covering critical infrastructure components.

---

## Test Suites

### 1. **TestFrameArena.cpp** (30 tests)
Frame-scoped bump allocator testing.

**Key Features Tested:**
- Allocation within capacity
- Alignment requirements (16, 32, 64 bytes)
- Reset and memory reuse
- Edge cases (zero size, exceeds capacity)
- Stress testing (1000 frame cycles)

**Example:**
```cpp
TEST(FrameArena, Allocate_Basic)
{
    FrameArena arena(1024);
    void* ptr = arena.Allocate(64);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(arena.Used(), 64);
}
```

---

### 2. **TestPoolAllocator.cpp** (35 tests)
Fixed-size object pool allocator testing.

**Key Features Tested:**
- Fixed-size block allocation
- Free list management (LIFO)
- Block reuse after deallocation
- Double-free detection
- Size/alignment mismatch handling

**Example:**
```cpp
TEST(PoolAllocator, Allocate_Single)
{
    PoolAllocator pool(64, 8, 10);
    void* ptr = pool.Allocate(64, 8);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(pool.AllocatedBlocks(), 1);
}
```

---

### 3. **TestSlabAllocator.cpp** (40 tests)
Versioned handle-based slab allocator testing.

**Key Features Tested:**
- **Handle versioning** (critical feature!)
- Slot reuse with version increment
- Old handle invalidation
- Object lifecycle (constructor/destructor)
- Complex object support

**Example:**
```cpp
TEST(SlabAllocator, HandleVersioning_AfterDeallocate)
{
    SlabAllocator<int> slab;
    Handle<int> handle1 = slab.Allocate();
    slab.Deallocate(handle1);
    
    Handle<int> handle2 = slab.Allocate();
    
    // Same index, different version
    EXPECT_EQ(handle1.Index(), handle2.Index());
    EXPECT_NE(handle1.Version(), handle2.Version());
    
    // Old handle is invalid
    EXPECT_FALSE(slab.IsValid(handle1));
}
```

---

### 4. **TestThreading.cpp** (35 tests)
Threading primitives and synchronization testing.

**Key Features Tested:**
- **Mutex**: Mutual exclusion, TryLock
- **LockGuard**: RAII, exception safety
- **RWLock**: Multiple readers, writer exclusion
- **Semaphore**: Counting, producer-consumer
- **ConditionVariable**: Wait/notify, timeouts
- **Thread Creation**: Priorities, naming

**Example:**
```cpp
TEST(Threading_Mutex, MutualExclusion)
{
    Mutex mutex;
    std::atomic<int> counter{0};
    
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < 1000; ++i) {
                mutex.Lock();
                counter.fetch_add(1);
                mutex.Unlock();
            }
        });
    }
    
    for (auto& t : threads) t.join();
    EXPECT_EQ(counter.load(), 4000);
}
```

---

## Test Statistics

| Test Suite | Tests | Lines | Focus Area |
|------------|-------|-------|------------|
| TestFrameArena | 30 | 432 | Bump allocation, reset |
| TestPoolAllocator | 35 | 483 | Fixed-size blocks, free list |
| TestSlabAllocator | 40 | 552 | **Versioned handles** |
| TestThreading | 35 | 448 | Synchronization primitives |
| **TOTAL** | **140** | **1,915** | **Core infrastructure** |

---

## Running Tests

### In CLion:
1. Right-click on test file → **Run**
2. Or click green arrow next to test name
3. View results in Run window

### Command Line:
```bash
# Build
cmake --build build --target TestFrameArena -j 8

# Run
./build/tests/unit/Core/TestFrameArena

# Run specific test
./build/tests/unit/Core/TestFrameArena --gtest_filter=FrameArena.Allocate_Basic
```

---

## Test Categories

### ✅ Basic Functionality
- Construction/destruction
- Allocation/deallocation
- Get/set operations

### ✅ Edge Cases
- Zero sizes
- Null pointers
- Capacity limits
- Invalid inputs

### ✅ Error Handling
- Out-of-memory
- Invalid alignments
- Double-free detection
- Invalid handle access

### ✅ Concurrency
- Thread safety verification
- Race condition detection
- Synchronization correctness

### ✅ Performance
- Stress tests (1000+ iterations)
- Many small allocations
- Random allocation patterns

### ✅ RAII & Exception Safety
- Resource cleanup
- Exception handling
- Scope-based locking

---

## Key Testing Patterns

### 1. Alignment Verification
```cpp
void* ptr = arena.Allocate(64, 16);
EXPECT_TRUE(IsAligned(ptr, 16));
```

### 2. Handle Versioning
```cpp
Handle<T> h1 = slab.Allocate();
slab.Deallocate(h1);
Handle<T> h2 = slab.Allocate();
EXPECT_NE(h1.Version(), h2.Version());
```

### 3. Concurrent Testing
```cpp
std::atomic<int> counter{0};
std::vector<std::thread> threads;
// Launch threads...
for (auto& t : threads) t.join();
EXPECT_EQ(counter.load(), expected);
```

### 4. RAII Exception Safety
```cpp
try {
    LockGuard guard(mutex);
    throw std::runtime_error("Test");
} catch (...) {}
// Mutex automatically unlocked
```

---

## Expected Results

All 140 tests should **PASS** ✅

```
[==========] Running 140 tests from 4 test suites.
[----------] Global test environment set-up.
[----------] 30 tests from FrameArena
[ RUN      ] FrameArena.Construction_ValidCapacity
[       OK ] FrameArena.Construction_ValidCapacity (0 ms)
...
[----------] 30 tests from FrameArena (X ms total)

[----------] 35 tests from PoolAllocator
...
[----------] 35 tests from PoolAllocator (X ms total)

[----------] 40 tests from SlabAllocator
...
[----------] 40 tests from SlabAllocator (X ms total)

[----------] 35 tests from Threading_*
...
[----------] 35 tests from Threading_* (X ms total)

[----------] Global test environment tear-down
[==========] 140 tests from 4 test suites ran. (X ms total)
[  PASSED  ] 140 tests.
```

---

## Troubleshooting

### Build Errors
- Ensure CMake is configured: `cmake -B build`
- Check dependencies: GTest should be available
- Verify Core module builds: `cmake --build build --target AthiVegam_Core`

### Test Failures
- **Alignment issues**: Platform-specific, may need adjustment
- **Timing issues**: Threading tests sensitive to system load
- **Memory issues**: Run with sanitizers for detailed info

### Debugging
1. Set breakpoint in test
2. Right-click → **Debug 'TestName'**
3. Step through with F8/F7
4. Inspect variables

---

## Coverage

### Memory Allocators: ✅ Excellent
- FrameArena: 30 tests
- PoolAllocator: 35 tests
- SlabAllocator: 40 tests

### Threading: ✅ Excellent
- Mutex: 6 tests
- LockGuard: 3 tests
- RWLock: 8 tests
- Semaphore: 6 tests
- ConditionVariable: 6 tests
- Thread Creation: 6 tests

### Total: ✅ 140 comprehensive tests

---

## Next Steps

1. **Build tests** in CLion
2. **Run all tests** (should take < 1 minute)
3. **Verify all pass** ✅
4. **Report any failures** with details

---

**Test Framework**: Google Test (GTest)  
**Build System**: CMake  
**Language**: C++17  
**Platform**: Windows (with cross-platform threading abstractions)

---

*Ready to run!* 🚀
