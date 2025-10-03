#include "Core/Platform/Time.hpp"

#include <windows.h>
#include <spdlog/spdlog.h>

namespace Engine::Time {

namespace {
    u64 _frequency = 0;
    bool _initialized = false;
}

void Initialize()
{
    if (_initialized)
    {
        spdlog::warn("Time::Initialize() called multiple times");
        return;
    }

    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq))
    {
        spdlog::error("Failed to query performance frequency");
        _frequency = 1; // Fallback to prevent division by zero
    }
    else
    {
        _frequency = static_cast<u64>(freq.QuadPart);
    }

    _initialized = true;
    spdlog::info("Time initialized successfully (frequency: {} Hz)", _frequency);
}

f64 GetTime()
{
    if (!_initialized)
    {
        Initialize();
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return static_cast<f64>(counter.QuadPart) / static_cast<f64>(_frequency);
}

f64 GetTimeMs()
{
    return GetTime() * 1000.0;
}

u64 GetTimeUs()
{
    if (!_initialized)
    {
        Initialize();
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (static_cast<u64>(counter.QuadPart) * 1000000ULL) / _frequency;
}

u64 GetPerformanceFrequency()
{
    if (!_initialized)
    {
        Initialize();
    }

    return _frequency;
}

u64 GetPerformanceCounter()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return static_cast<u64>(counter.QuadPart);
}

// Timer implementation

void Timer::Start()
{
    _startTime = GetPerformanceCounter();
    _running = true;
}

void Timer::Stop()
{
    if (_running)
    {
        _stopTime = GetPerformanceCounter();
        _running = false;
    }
}

void Timer::Reset()
{
    _startTime = 0;
    _stopTime = 0;
    _running = false;
}

f64 Timer::ElapsedSeconds() const
{
    u64 endTime = _running ? GetPerformanceCounter() : _stopTime;
    u64 elapsed = endTime - _startTime;
    return static_cast<f64>(elapsed) / static_cast<f64>(GetPerformanceFrequency());
}

f64 Timer::ElapsedMs() const
{
    return ElapsedSeconds() * 1000.0;
}

u64 Timer::ElapsedUs() const
{
    u64 endTime = _running ? GetPerformanceCounter() : _stopTime;
    u64 elapsed = endTime - _startTime;
    return (elapsed * 1000000ULL) / GetPerformanceFrequency();
}

// DeltaTime implementation

void DeltaTime::Update()
{
    f64 currentTime = GetTime();

    if (_lastTime == 0.0)
    {
        // First frame
        _lastTime = currentTime;
        _deltaSeconds = 0.0f;
        _smoothedDelta = 0.0f;
        _fps = 0.0f;
        return;
    }

    // Calculate raw delta time
    f64 delta = currentTime - _lastTime;
    _deltaSeconds = static_cast<f32>(delta);

    // Clamp delta time to prevent huge jumps (e.g., during debugging)
    constexpr f32 maxDelta = 0.1f; // 100ms max
    if (_deltaSeconds > maxDelta)
    {
        _deltaSeconds = maxDelta;
    }

    // Exponential smoothing for stable FPS display
    if (_smoothedDelta == 0.0f)
    {
        _smoothedDelta = _deltaSeconds;
    }
    else
    {
        _smoothedDelta = _smoothedDelta * (1.0f - _smoothingFactor) + _deltaSeconds * _smoothingFactor;
    }

    // Calculate FPS from smoothed delta
    if (_smoothedDelta > 0.0f)
    {
        _fps = 1.0f / _smoothedDelta;
    }

    // Update state
    _lastTime = currentTime;
    _totalTime += _deltaSeconds;
    _frameCount++;
}

} // namespace Engine::Time

