# Quick Test Running Guide

## 🚀 Running Tests in CLion

### Method 1: Run All Tests
1. Open CLion
2. Go to **Run → Edit Configurations**
3. Add **CTest** configuration
4. Click **Run** (green play button)

### Method 2: Run Individual Test File
1. Navigate to `tests/unit/Core/TestFrameArena.cpp`
2. Right-click on the file
3. Select **Run 'TestFrameArena'**
4. View results in Run window

### Method 3: Run Specific Test
1. Open test file (e.g., `TestFrameArena.cpp`)
2. Find test function (e.g., `TEST(FrameArena, Allocate_Basic)`)
3. Click green arrow next to test name
4. Or right-click → **Run 'FrameArena.Allocate_Basic'**

### Method 4: Run from CMake Tool Window
1. Open **View → Tool Windows → CMake**
2. Expand **Tests** node
3. Right-click on test → **Run**

---

## 📊 Expected Test Results

### All Tests Should Pass ✅

```
[==========] Running 140 tests from 4 test suites.
[----------] 30 tests from FrameArena
[----------] 35 tests from PoolAllocator
[----------] 40 tests from SlabAllocator
[----------] 35 tests from Threading_*
[==========] 140 tests from 4 test suites ran.
[  PASSED  ] 140 tests.
```

---

## 🐛 If Tests Fail

### Report the Following:
1. **Test name** (e.g., `FrameArena.Allocate_Basic`)
2. **Error message** (copy from output)
3. **Expected vs Actual** values
4. **Stack trace** (if available)

### Example Failure Report:
```
Test: FrameArena.Allocate_Basic
Error: Expected: (ptr) != (nullptr), actual: NULL
File: TestFrameArena.cpp:45
```

---

## 🔍 Debugging Failed Tests

### In CLion:
1. Set breakpoint in test
2. Right-click test → **Debug 'TestName'**
3. Step through code with F8/F7
4. Inspect variables in Debug window

### Common Issues:
- **Alignment failures**: Platform-specific alignment differences
- **Timing issues**: Threading tests may be sensitive to system load
- **Memory issues**: Check for leaks with Valgrind/sanitizers

---

## 📝 Test Files Created

| File | Tests | Purpose |
|------|-------|---------|
| `TestFrameArena.cpp` | 30 | Frame-scoped bump allocator |
| `TestPoolAllocator.cpp` | 35 | Fixed-size object pool |
| `TestSlabAllocator.cpp` | 40 | Versioned handle allocator |
| `TestThreading.cpp` | 35 | Threading primitives |

---

## ✅ Checklist

- [ ] Build project successfully
- [ ] Run TestFrameArena (30 tests)
- [ ] Run TestPoolAllocator (35 tests)
- [ ] Run TestSlabAllocator (40 tests)
- [ ] Run TestThreading (35 tests)
- [ ] All 140 tests pass
- [ ] Report any failures

---

## 🎯 Quick Commands (if using terminal)

```bash
# Build all tests
cmake --build build -j 8

# Run all tests
cd build && ctest --output-on-failure

# Run specific test suite
./build/tests/unit/Core/TestFrameArena

# Run with filter
./build/tests/unit/Core/TestFrameArena --gtest_filter=FrameArena.Allocate_*

# Run with verbose output
./build/tests/unit/Core/TestFrameArena --gtest_verbose
```

---

**Ready to test!** 🚀

Please run the tests and report back the results!
