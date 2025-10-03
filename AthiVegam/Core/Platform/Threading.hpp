#pragma once

#pragma once

#include "Core/Types.hpp"
#include <functional>

/// @file Threading.hpp
/// @brief Threading primitives and synchronization

namespace Engine::Threading {

/// @brief Thread handle (opaque)
using ThreadHandle = void*;

/// @brief Invalid thread handle constant
inline constexpr ThreadHandle InvalidThreadHandle = nullptr;

/// @brief Thread function signature
using ThreadFunction = std::function<void()>;

/// @brief Thread priority levels
enum class ThreadPriority {
    Low,
    Normal,
    High,
    Critical
};

/// @brief Create and start a new thread
/// @param fn Function to execute on the thread
/// @param priority Thread priority
/// @return Thread handle or error
ThreadHandle CreateThread(ThreadFunction fn, ThreadPriority priority = ThreadPriority::Normal);

/// @brief Wait for a thread to complete
/// @param handle Thread handle
void JoinThread(ThreadHandle handle);

/// @brief Detach a thread (runs independently)
/// @param handle Thread handle
void DetachThread(ThreadHandle handle);

/// @brief Get the current thread's ID
/// @return Thread ID
u64 GetCurrentThreadId();

/// @brief Set the current thread's name (for debugging)
/// @param name Thread name
void SetCurrentThreadName(const char* name);

/// @brief Sleep the current thread
/// @param milliseconds Time to sleep in milliseconds
void Sleep(u32 milliseconds);

/// @brief Yield the current thread's time slice
void YieldThread();

/// @brief Mutex for mutual exclusion
class Mutex {
public:
    Mutex();
    ~Mutex();

    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    /// @brief Lock the mutex
    void Lock();

    /// @brief Try to lock the mutex without blocking
    /// @return true if lock was acquired
    bool TryLock();

    /// @brief Unlock the mutex
    void Unlock();

private:
    friend class ConditionVariable;
    void* _handle;
};

/// @brief RAII lock guard for Mutex
class LockGuard {
public:
    explicit LockGuard(Mutex& mutex) : _mutex(mutex) {
        _mutex.Lock();
    }
    
    ~LockGuard() {
        _mutex.Unlock();
    }
    
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
    
private:
    Mutex& _mutex;
};

/// @brief Condition variable for thread synchronization
class ConditionVariable {
public:
    ConditionVariable();
    ~ConditionVariable();
    
    ConditionVariable(const ConditionVariable&) = delete;
    ConditionVariable& operator=(const ConditionVariable&) = delete;
    
    /// @brief Wait for notification
    /// @param mutex Mutex to unlock while waiting
    void Wait(Mutex& mutex);
    
    /// @brief Wait for notification with timeout
    /// @param mutex Mutex to unlock while waiting
    /// @param timeoutMs Timeout in milliseconds
    /// @return true if notified, false if timeout
    bool WaitFor(Mutex& mutex, u32 timeoutMs);
    
    /// @brief Notify one waiting thread
    void NotifyOne();
    
    /// @brief Notify all waiting threads
    void NotifyAll();
    
private:
    void* _handle;
};

/// @brief Read-write lock for shared/exclusive access
class RWLock {
public:
    RWLock();
    ~RWLock();
    
    RWLock(const RWLock&) = delete;
    RWLock& operator=(const RWLock&) = delete;
    
    /// @brief Acquire shared (read) lock
    void LockShared();
    
    /// @brief Try to acquire shared (read) lock
    /// @return true if lock was acquired
    bool TryLockShared();
    
    /// @brief Release shared (read) lock
    void UnlockShared();
    
    /// @brief Acquire exclusive (write) lock
    void LockExclusive();
    
    /// @brief Try to acquire exclusive (write) lock
    /// @return true if lock was acquired
    bool TryLockExclusive();
    
    /// @brief Release exclusive (write) lock
    void UnlockExclusive();
    
private:
    void* _handle;
};

/// @brief Semaphore for counting synchronization
class Semaphore {
public:
    /// @brief Create semaphore with initial count
    /// @param initialCount Initial count value
    explicit Semaphore(u32 initialCount = 0);
    ~Semaphore();
    
    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;
    
    /// @brief Wait (decrement) the semaphore
    void Wait();
    
    /// @brief Try to wait without blocking
    /// @return true if wait succeeded
    bool TryWait();
    
    /// @brief Signal (increment) the semaphore
    void Signal();
    
private:
    void* _handle;
};

} // namespace Engine::Threading

