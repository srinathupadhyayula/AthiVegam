@echo off
REM AthiVegam - Job System Performance Benchmark Runner
REM This script runs all performance tests and outputs results

echo ========================================
echo AthiVegam Job System Performance Tests
echo ========================================
echo.

set BUILD_DIR=build\debug
set TEST_EXE=%BUILD_DIR%\bin\Debug\JobsTests.exe

if not exist %TEST_EXE% (
    echo ERROR: Test executable not found at %TEST_EXE%
    echo Please build the project first.
    exit /b 1
)

echo Running performance benchmarks...
echo.

echo ----------------------------------------
echo 1. Work-Stealing Performance Tests
echo ----------------------------------------
%TEST_EXE% --gtest_filter=WorkStealingTest.*
echo.

echo ----------------------------------------
echo 2. ParallelFor Scaling Tests
echo ----------------------------------------
%TEST_EXE% --gtest_filter=ParallelForTest.LargeArrayPerformance
echo.
%TEST_EXE% --gtest_filter=ParallelForTest.ScalingBenchmark
echo.
%TEST_EXE% --gtest_filter=ParallelForTest.OptimalGrainSize
echo.
%TEST_EXE% --gtest_filter=ParallelForTest.MemoryBandwidth
echo.

echo ----------------------------------------
echo Performance benchmarks complete!
echo ----------------------------------------
echo.
echo Results saved above. Copy the output to update Phase2_Completion_Report.md
echo with actual measured performance numbers.

