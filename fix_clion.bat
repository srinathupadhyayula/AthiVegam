@echo off
echo ========================================
echo CLion Build Fix Script
echo ========================================
echo.
echo This script will:
echo 1. Clean build directories
echo 2. Reconfigure CMake with Visual Studio generator
echo 3. Build the project
echo.
echo IMPORTANT: Close CLion before running this script!
echo.
pause

echo.
echo Step 1: Cleaning build directories...
if exist "build" rmdir /s /q "build"
if exist "cmake-build-debug" rmdir /s /q "cmake-build-debug"
echo Done.

echo.
echo Step 2: Configuring CMake with Visual Studio generator...
call "E:\Software\VisualStudio\Common7\Tools\VsDevCmd.bat"
cmake --preset windows-debug
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)
echo Done.

echo.
echo Step 3: Building project...
cmake --build build/debug --config Debug
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)
echo Done.

echo.
echo ========================================
echo SUCCESS! Build completed.
echo ========================================
echo.
echo Now you can:
echo 1. Open CLion
echo 2. File -^> Reload CMake Project
echo 3. Build in CLion (Ctrl+F9)
echo.
pause
