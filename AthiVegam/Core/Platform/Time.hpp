#pragma once

#include "Core/Types.hpp"

/// @file Time.hpp
/// @brief High-resolution time and delta time management

namespace Engine::Time {

/// @brief Initialize the time system
void Initialize();

/// @brief Get the current time in seconds since engine start
/// @return Time in seconds
f64 GetTime();

/// @brief Get the current time in milliseconds since engine start
/// @return Time in milliseconds
f64 GetTimeMs();

/// @brief Get the current time in microseconds since engine start
/// @return Time in microseconds
u64 GetTimeUs();

/// @brief Get the high-resolution performance counter frequency
/// @return Frequency in counts per second
u64 GetPerformanceFrequency();

/// @brief Get the current performance counter value
/// @return Performance counter value
u64 GetPerformanceCounter();

/// @brief Timer class for measuring elapsed time
class Timer {
public:
    /// @brief Start or restart the timer
    void Start();

    /// @brief Stop the timer
    void Stop();

    /// @brief Reset the timer to zero
    void Reset();

    /// @brief Get elapsed time in seconds
    /// @return Elapsed time in seconds
    f64 ElapsedSeconds() const;

    /// @brief Get elapsed time in milliseconds
    /// @return Elapsed time in milliseconds
    f64 ElapsedMs() const;

    /// @brief Get elapsed time in microseconds
    /// @return Elapsed time in microseconds
    u64 ElapsedUs() const;

    /// @brief Check if the timer is currently running
    /// @return true if running
    bool IsRunning() const { return _running; }

private:
    u64 _startTime = 0;
    u64 _stopTime = 0;
    bool _running = false;
};

/// @brief Delta time tracker for frame timing
class DeltaTime {
public:
    /// @brief Update delta time (call once per frame)
    void Update();

    /// @brief Get delta time in seconds
    /// @return Delta time in seconds
    f32 DeltaSeconds() const { return _deltaSeconds; }

    /// @brief Get delta time in milliseconds
    /// @return Delta time in milliseconds
    f32 DeltaMs() const { return _deltaSeconds * 1000.0f; }

    /// @brief Get smoothed delta time (averaged over multiple frames)
    /// @return Smoothed delta time in seconds
    f32 SmoothedDeltaSeconds() const { return _smoothedDelta; }

    /// @brief Get current frame rate
    /// @return Frames per second
    f32 Fps() const { return _fps; }

    /// @brief Get total elapsed time since start
    /// @return Total time in seconds
    f64 TotalTime() const { return _totalTime; }

    /// @brief Get frame count
    /// @return Number of frames since start
    u64 FrameCount() const { return _frameCount; }

private:
    f64 _lastTime = 0.0;
    f32 _deltaSeconds = 0.0f;
    f32 _smoothedDelta = 0.0f;
    f32 _fps = 0.0f;
    f64 _totalTime = 0.0;
    u64 _frameCount = 0;

    static constexpr f32 _smoothingFactor = 0.1f;
};

} // namespace Engine::Time

