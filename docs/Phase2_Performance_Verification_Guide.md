# Phase 2: Job System - Performance Verification Guide

This guide explains how to measure and verify the actual performance of the Job System on your machine.

## Quick Start

### Option 1: Run Performance Script (Windows)

```bash
# From project root
scripts\run_performance_benchmarks.bat
```

This will run all performance tests and output results to the console.

### Option 2: Run via CTest (Cross-platform)

```bash
cd build/debug
ctest -C Debug -V -R "WorkStealing|ParallelFor"
```

The `-V` flag shows verbose output including performance metrics.

### Option 3: Run Individual Tests in CLion

1. Open any test file (e.g., `TestWorkStealing.cpp`)
2. Find a performance test (e.g., `TEST_F(WorkStealingTest, ParallelSpeedup)`)
3. Click the green ▶️ icon next to the test
4. View output in the Run window

---

## Performance Metrics to Collect

### 1. Parallel Speedup

**Test:** `WorkStealingTest.ParallelSpeedup`

**What it measures:** How much faster parallel execution is vs sequential

**Expected output:**
```
[Performance] Parallel speedup (1000 jobs, 10000 work per job):
  Sequential: 523 ms
  Parallel: 87 ms
  Speedup: 6.01x
  Efficiency: 75.1% (on 8 cores)
```

**What to look for:**
- **Speedup:** Should be > 1.0x (parallel faster than sequential)
- **Efficiency:** Speedup / Core Count (ideally 70-90%)
- **On 8 cores:** Expect 5-7x speedup (62-87% efficiency)

---

### 2. Job Submission Overhead

**Test:** `WorkStealingTest.JobSubmissionOverhead`

**What it measures:** Time to submit a job to the scheduler

**Expected output:**
```
[Performance] Job submission overhead (10000 jobs):
  Total time: 156 ms
  Average per job: 15.6 μs
```

**What to look for:**
- **Average per job:** Should be < 100μs
- **Typical values:** 10-50μs depending on CPU

---

### 3. Work-Stealing Efficiency

**Test:** `WorkStealingTest.WorkStealingEfficiency`

**What it measures:** How often jobs are stolen vs executed locally

**Expected output:**
```
[Performance] Work-stealing efficiency (1000 jobs):
  Jobs executed: 1000
  Jobs stolen: 342
  Steal ratio: 34.2%
```

**What to look for:**
- **Steal ratio:** 20-50% indicates good load balancing
- **Too low (<10%):** Work not distributed evenly
- **Too high (>70%):** Excessive stealing overhead

---

### 4. ParallelFor Scaling

**Test:** `ParallelForTest.ScalingBenchmark`

**What it measures:** How ParallelFor performance scales with data size

**Expected output:**
```
[Performance] Scaling benchmark:
  Size 1000: 0.5 ms (0.5 μs/element)
  Size 10000: 3.2 ms (0.32 μs/element)
  Size 100000: 28.7 ms (0.287 μs/element)
  Size 1000000: 245.3 ms (0.245 μs/element)
```

**What to look for:**
- **Time per element:** Should decrease or stay constant as size increases
- **Indicates:** Good cache utilization and parallelism

---

### 5. Large Array Performance

**Test:** `ParallelForTest.LargeArrayPerformance`

**What it measures:** Parallel vs sequential for 1M element array

**Expected output:**
```
[Performance] Large array (1M elements):
  Sequential: 45 ms
  Parallel: 12 ms
  Speedup: 3.75x
```

**What to look for:**
- **Speedup:** Should be > 1.0x
- **Note:** Speedup depends on workload complexity
- **Heavier computation:** Better speedup

---

### 6. Optimal Grain Size

**Test:** `ParallelForTest.OptimalGrainSize`

**What it measures:** How grain size affects performance

**Expected output:**
```
[Performance] Grain size impact (100K elements):
  Grain 10: 32.5 ms
  Grain 100: 18.7 ms
  Grain 1000: 15.2 ms
  Grain 10000: 16.8 ms
```

**What to look for:**
- **Sweet spot:** Usually 100-1000 elements per job
- **Too small:** Overhead dominates
- **Too large:** Poor load balancing

---

### 7. Memory Bandwidth

**Test:** `ParallelForTest.MemoryBandwidth`

**What it measures:** Memory throughput during parallel operations

**Expected output:**
```
[Performance] Memory bandwidth:
  Time: 234 ms
  Bandwidth: 683.5 MB/s
```

**What to look for:**
- **Bandwidth:** Depends on your RAM speed
- **Typical DDR4:** 500-2000 MB/s per thread
- **Parallel:** Should be higher than single-threaded

---

## Updating the Completion Report

After running the benchmarks, update `docs/Phase2_Completion_Report.md` with your actual numbers:

1. Run the performance script or tests
2. Copy the output
3. Replace the "Performance Results" section in the report with your actual measurements
4. Include your system specs (CPU model, core count, RAM speed)

### Example Update:

```markdown
## Performance Results

**System Specs:**
- CPU: Intel Core i7-9700K (8 cores, 8 threads)
- RAM: 32GB DDR4-3200
- OS: Windows 11
- Compiler: MSVC 19.44

**Measured Performance:**

#### Parallel Speedup
- Sequential time: 523ms (1000 jobs, 10K work per job)
- Parallel time: 87ms (8 worker threads)
- Speedup: 6.01x
- Efficiency: 75.1%

#### Job Submission Overhead
- Average: 15.6μs per job (10,000 jobs)
- Total time: 156ms

... (continue with other metrics)
```

---

## Troubleshooting

### Low Speedup (<2x on 8 cores)

**Possible causes:**
- Workload too light (overhead dominates)
- Debug build (use Release build for accurate measurements)
- Background processes consuming CPU
- Thermal throttling

**Solutions:**
- Build in Release mode: `cmake --preset windows-release`
- Close background applications
- Increase workload complexity in tests

### High Variance in Results

**Possible causes:**
- Background processes
- Turbo boost/frequency scaling
- Thermal throttling

**Solutions:**
- Run tests multiple times and average
- Disable turbo boost for consistent results
- Ensure adequate cooling

### Tests Timing Out

**Possible causes:**
- Very slow CPU
- Debug build with heavy instrumentation

**Solutions:**
- Increase test timeout in CMakeLists.txt
- Use Release build

---

## Best Practices

1. **Use Release builds** for performance measurements
2. **Run multiple times** and average results
3. **Close background apps** to reduce noise
4. **Document system specs** when reporting numbers
5. **Compare before/after** when optimizing

---

## Next Steps

After collecting real performance data:

1. Update `Phase2_Completion_Report.md` with actual numbers
2. Commit the updated report
3. If performance doesn't meet targets, investigate bottlenecks
4. Consider profiling with tools like Tracy or Optick

---

**Note:** The numbers in the original completion report were **estimated examples**, not actual measurements. Always verify with real data from your system!

