// AthiVegam Engine - Threading Primitives Unit Tests
// License: MIT

#include <gtest/gtest.h>
#include "Core/Platform/Threading.hpp"
#include "Core/Types.hpp"
#include <atomic>
#include <vector>
#include <chrono>
#include <thread>

using namespace Engine::Threading;
using namespace Engine;

// ============================================================================
// Mutex Tests
// ============================================================================

TEST(Threading_Mutex, Construction)
{
    Mutex mutex;
    // Should construct without errors
    SUCCEED();
}

TEST(Threading_Mutex, LockUnlock_Basic)
{
    Mutex mutex;
    
    mutex.Lock();
    mutex.Unlock();
    
    SUCCEED();
}

TEST(Threading_Mutex, LockUnlock_Multiple)
{
    Mutex mutex;
    
    for (int i = 0; i < 100; ++i)
    {
        mutex.Lock();
        mutex.Unlock();
    }
    
    SUCCEED();
}

TEST(Threading_Mutex, TryLock_Success)
{
    Mutex mutex;
    
    bool locked = mutex.TryLock();
    
    EXPECT_TRUE(locked);
    
    if (locked)
    {
        mutex.Unlock();
    }
}

TEST(Threading_Mutex, TryLock_Failure)
{
    Mutex mutex;
    
    mutex.Lock();
    
    // Try to lock from same thread (should fail on most implementations)
    bool locked = mutex.TryLock();
    (void)locked; // Documented: behavior may vary by platform
    
    // Note: Behavior may vary by platform
    // On Windows, recursive locking typically fails
    
    mutex.Unlock();
}

TEST(Threading_Mutex, MutualExclusion)
{
    Mutex mutex;
    std::atomic<int> counter{0};
    std::atomic<int> maxConcurrent{0};
    std::atomic<int> currentConcurrent{0};
    
    constexpr int numThreads = 4;
    constexpr int incrementsPerThread = 1000;
    
    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; ++t)
    {
        threads.emplace_back([&]() {
            for (int i = 0; i < incrementsPerThread; ++i)
            {
                mutex.Lock();
                
                // Track concurrent access
                int concurrent = currentConcurrent.fetch_add(1, std::memory_order_relaxed) + 1;
                int expected = maxConcurrent.load(std::memory_order_relaxed);
                while (concurrent > expected && 
                       !maxConcurrent.compare_exchange_weak(expected, concurrent, std::memory_order_relaxed))
                {
                    expected = maxConcurrent.load(std::memory_order_relaxed);
                }
                
                // Critical section
                counter.fetch_add(1, std::memory_order_relaxed);
                
                currentConcurrent.fetch_sub(1, std::memory_order_relaxed);
                mutex.Unlock();
            }
        });
    }
    
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    // All increments should have been counted
    EXPECT_EQ(counter.load(), numThreads * incrementsPerThread);
    
    // Mutex should ensure only one thread in critical section
    EXPECT_EQ(maxConcurrent.load(), 1);
}

// ============================================================================
// LockGuard Tests
// ============================================================================

TEST(Threading_LockGuard, RAII_Basic)
{
    Mutex mutex;
    
    {
        LockGuard guard(mutex);
        // Mutex is locked
    }
    // Mutex should be unlocked after guard destruction
    
    // Should be able to lock again
    bool locked = mutex.TryLock();
    EXPECT_TRUE(locked);
    if (locked)
    {
        mutex.Unlock();
    }
}

TEST(Threading_LockGuard, RAII_Exception)
{
    Mutex mutex;
    std::atomic<bool> mutexLocked{false};
    
    try
    {
        LockGuard guard(mutex);
        mutexLocked = true;
        throw std::runtime_error("Test exception");
    }
    catch (...)
    {
        // Exception caught
    }
    
    EXPECT_TRUE(mutexLocked.load());
    
    // Mutex should be unlocked despite exception
    bool locked = mutex.TryLock();
    EXPECT_TRUE(locked);
    if (locked)
    {
        mutex.Unlock();
    }
}

TEST(Threading_LockGuard, MutualExclusion)
{
    Mutex mutex;
    std::atomic<int> counter{0};
    
    constexpr int numThreads = 4;
    constexpr int incrementsPerThread = 1000;
    
    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; ++t)
    {
        threads.emplace_back([&]() {
            for (int i = 0; i < incrementsPerThread; ++i)
            {
                LockGuard guard(mutex);
                counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    EXPECT_EQ(counter.load(), numThreads * incrementsPerThread);
}

// ============================================================================
// RWLock Tests
// ============================================================================

TEST(Threading_RWLock, Construction)
{
    RWLock rwlock;
    SUCCEED();
}

TEST(Threading_RWLock, LockShared_Basic)
{
    RWLock rwlock;
    
    rwlock.LockShared();
    rwlock.UnlockShared();
    
    SUCCEED();
}

TEST(Threading_RWLock, LockExclusive_Basic)
{
    RWLock rwlock;
    
    rwlock.LockExclusive();
    rwlock.UnlockExclusive();
    
    SUCCEED();
}

TEST(Threading_RWLock, TryLockShared)
{
    RWLock rwlock;
    
    bool locked = rwlock.TryLockShared();
    EXPECT_TRUE(locked);
    
    if (locked)
    {
        rwlock.UnlockShared();
    }
}

TEST(Threading_RWLock, TryLockExclusive)
{
    RWLock rwlock;
    
    bool locked = rwlock.TryLockExclusive();
    EXPECT_TRUE(locked);
    
    if (locked)
    {
        rwlock.UnlockExclusive();
    }
}

TEST(Threading_RWLock, MultipleReaders)
{
    RWLock rwlock;
    std::atomic<int> concurrentReaders{0};
    std::atomic<int> maxConcurrentReaders{0};
    
    constexpr int numReaders = 8;
    
    std::vector<std::thread> threads;
    for (int t = 0; t < numReaders; ++t)
    {
        threads.emplace_back([&]() {
            rwlock.LockShared();
            
            int concurrent = concurrentReaders.fetch_add(1, std::memory_order_relaxed) + 1;
            int expected = maxConcurrentReaders.load(std::memory_order_relaxed);
            while (concurrent > expected && 
                   !maxConcurrentReaders.compare_exchange_weak(expected, concurrent, std::memory_order_relaxed))
            {
                expected = maxConcurrentReaders.load(std::memory_order_relaxed);
            }
            
            // Simulate read operation
            Sleep(10);
            
            concurrentReaders.fetch_sub(1, std::memory_order_relaxed);
            rwlock.UnlockShared();
        });
    }
    
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    // Multiple readers should have been concurrent
    EXPECT_GT(maxConcurrentReaders.load(), 1);
}

TEST(Threading_RWLock, WriterBlocksReaders)
{
    RWLock rwlock;
    std::atomic<bool> writerActive{false};
    std::atomic<bool> readerBlockedByWriter{false};
    
    // Writer thread
    std::thread writer([&]() {
        rwlock.LockExclusive();
        writerActive = true;
        Sleep(100); // Hold lock for a bit
        writerActive = false;
        rwlock.UnlockExclusive();
    });
    
    // Wait for writer to acquire lock
    Sleep(20);
    
    // Reader thread
    std::thread reader([&]() {
        if (writerActive.load())
        {
            readerBlockedByWriter = true;
        }
        rwlock.LockShared();
        // If we get here, writer should be done
        EXPECT_FALSE(writerActive.load());
        rwlock.UnlockShared();
    });
    
    writer.join();
    reader.join();
    
    EXPECT_TRUE(readerBlockedByWriter.load());
}

TEST(Threading_RWLock, ReaderBlocksWriter)
{
    RWLock rwlock;
    std::atomic<bool> readerActive{false};
    std::atomic<bool> writerBlockedByReader{false};
    
    // Reader thread
    std::thread reader([&]() {
        rwlock.LockShared();
        readerActive = true;
        Sleep(100); // Hold lock for a bit
        readerActive = false;
        rwlock.UnlockShared();
    });
    
    // Wait for reader to acquire lock
    Sleep(20);
    
    // Writer thread
    std::thread writer([&]() {
        if (readerActive.load())
        {
            writerBlockedByReader = true;
        }
        rwlock.LockExclusive();
        // If we get here, reader should be done
        EXPECT_FALSE(readerActive.load());
        rwlock.UnlockExclusive();
    });
    
    reader.join();
    writer.join();
    
    EXPECT_TRUE(writerBlockedByReader.load());
}

// ============================================================================
// Semaphore Tests
// ============================================================================

TEST(Threading_Semaphore, Construction_ZeroCount)
{
    Semaphore sem(0);
    SUCCEED();
}

TEST(Threading_Semaphore, Construction_NonZeroCount)
{
    Semaphore sem(5);
    SUCCEED();
}

TEST(Threading_Semaphore, Signal_Wait_Basic)
{
    Semaphore sem(0);
    
    std::thread signaler([&]() {
        Sleep(50);
        sem.Signal();
    });
    
    sem.Wait(); // Should block until signaled
    
    signaler.join();
    SUCCEED();
}

TEST(Threading_Semaphore, TryWait_Success)
{
    Semaphore sem(1);
    
    bool acquired = sem.TryWait();
    
    EXPECT_TRUE(acquired);
}

TEST(Threading_Semaphore, TryWait_Failure)
{
    Semaphore sem(0);
    
    bool acquired = sem.TryWait();
    
    EXPECT_FALSE(acquired);
}

TEST(Threading_Semaphore, MultipleSignals)
{
    Semaphore sem(0);
    
    // Signal 3 times
    sem.Signal();
    sem.Signal();
    sem.Signal();
    
    // Should be able to wait 3 times without blocking
    bool w1 = sem.TryWait();
    bool w2 = sem.TryWait();
    bool w3 = sem.TryWait();
    bool w4 = sem.TryWait();
    
    EXPECT_TRUE(w1);
    EXPECT_TRUE(w2);
    EXPECT_TRUE(w3);
    EXPECT_FALSE(w4); // Fourth should fail
}

TEST(Threading_Semaphore, ProducerConsumer)
{
    Semaphore sem(0);
    std::atomic<int> producedCount{0};
    std::atomic<int> consumedCount{0};
    
    constexpr int itemCount = 100;
    
    // Producer
    std::thread producer([&]() {
        for (int i = 0; i < itemCount; ++i)
        {
            producedCount.fetch_add(1, std::memory_order_relaxed);
            sem.Signal();
            Sleep(1);
        }
    });
    
    // Consumer
    std::thread consumer([&]() {
        for (int i = 0; i < itemCount; ++i)
        {
            sem.Wait();
            consumedCount.fetch_add(1, std::memory_order_relaxed);
        }
    });
    
    producer.join();
    consumer.join();
    
    EXPECT_EQ(producedCount.load(), itemCount);
    EXPECT_EQ(consumedCount.load(), itemCount);
}

// ============================================================================
// ConditionVariable Tests
// ============================================================================

TEST(Threading_ConditionVariable, Construction)
{
    ConditionVariable cv;
    SUCCEED();
}

TEST(Threading_ConditionVariable, NotifyOne_Basic)
{
    Mutex mutex;
    ConditionVariable cv;
    std::atomic<bool> notified{false};
    
    std::thread waiter([&]() {
        mutex.Lock();
        cv.Wait(mutex);
        notified = true;
        mutex.Unlock();
    });
    
    // Give waiter time to start waiting
    Sleep(50);
    
    cv.NotifyOne();
    
    waiter.join();
    
    EXPECT_TRUE(notified.load());
}

TEST(Threading_ConditionVariable, NotifyAll_MultipleWaiters)
{
    Mutex mutex;
    ConditionVariable cv;
    std::atomic<int> notifiedCount{0};
    
    constexpr int numWaiters = 5;
    
    std::vector<std::thread> waiters;
    for (int i = 0; i < numWaiters; ++i)
    {
        waiters.emplace_back([&]() {
            mutex.Lock();
            cv.Wait(mutex);
            notifiedCount.fetch_add(1, std::memory_order_relaxed);
            mutex.Unlock();
        });
    }
    
    // Give waiters time to start waiting
    Sleep(100);
    
    cv.NotifyAll();
    
    for (auto& waiter : waiters)
    {
        waiter.join();
    }
    
    EXPECT_EQ(notifiedCount.load(), numWaiters);
}

TEST(Threading_ConditionVariable, WaitFor_Timeout)
{
    Mutex mutex;
    ConditionVariable cv;
    
    mutex.Lock();
    bool notified = cv.WaitFor(mutex, 50); // 50ms timeout
    mutex.Unlock();
    
    EXPECT_FALSE(notified); // Should timeout
}

TEST(Threading_ConditionVariable, WaitFor_Notified)
{
    Mutex mutex;
    ConditionVariable cv;
    std::atomic<bool> result{false};
    
    std::thread waiter([&]() {
        mutex.Lock();
        bool notified = cv.WaitFor(mutex, 1000); // 1 second timeout
        result = notified;
        mutex.Unlock();
    });
    
    // Notify before timeout
    Sleep(50);
    cv.NotifyOne();
    
    waiter.join();
    
    EXPECT_TRUE(result.load());
}

TEST(Threading_ConditionVariable, ProducerConsumerPattern)
{
    Mutex mutex;
    ConditionVariable cv;
    std::vector<int> queue;
    std::atomic<bool> done{false};
    
    constexpr int itemCount = 100;
    
    // Producer
    std::thread producer([&]() {
        for (int i = 0; i < itemCount; ++i)
        {
            {
                LockGuard guard(mutex);
                queue.push_back(i);
            }
            cv.NotifyOne();
            Sleep(1);
        }
        done = true;
        cv.NotifyOne();
    });
    
    // Consumer
    std::thread consumer([&]() {
        int consumed = 0;
        while (consumed < itemCount)
        {
            mutex.Lock();
            while (queue.empty() && !done.load())
            {
                cv.Wait(mutex);
            }
            
            if (!queue.empty())
            {
                queue.erase(queue.begin());
                consumed++;
            }
            mutex.Unlock();
        }
    });
    
    producer.join();
    consumer.join();
    
    EXPECT_TRUE(queue.empty());
}

// ============================================================================
// Thread Creation Tests
// ============================================================================

TEST(Threading_Thread, CreateAndJoin)
{
    std::atomic<bool> executed{false};
    
    ThreadHandle handle = CreateThread([&]() {
        executed = true;
    });
    
    ASSERT_NE(handle, InvalidThreadHandle);
    
    JoinThread(handle);
    
    EXPECT_TRUE(executed.load());
}

TEST(Threading_Thread, CreateWithPriority)
{
    std::atomic<bool> executed{false};
    
    ThreadHandle handle = CreateThread([&]() {
        executed = true;
    }, ThreadPriority::High);
    
    ASSERT_NE(handle, InvalidThreadHandle);
    
    JoinThread(handle);
    
    EXPECT_TRUE(executed.load());
}

TEST(Threading_Thread, GetCurrentThreadId)
{
    u64 mainThreadId = GetCurrentThreadId();
    
    std::atomic<u64> childThreadId{static_cast<u64>(0)};
    
    ThreadHandle handle = CreateThread([&]() {
        childThreadId = GetCurrentThreadId();
    });
    
    JoinThread(handle);
    
    EXPECT_NE(mainThreadId, static_cast<u64>(0));
    EXPECT_NE(childThreadId.load(), static_cast<u64>(0));
    EXPECT_NE(mainThreadId, childThreadId.load());
}

TEST(Threading_Thread, Sleep)
{
    auto start = std::chrono::high_resolution_clock::now();
    
    Sleep(100); // 100ms
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should sleep at least 100ms (allow some tolerance)
    EXPECT_GE(duration.count(), 90);
}

TEST(Threading_Thread, YieldThread)
{
    // Just ensure it doesn't crash
    YieldThread();
    SUCCEED();
}

TEST(Threading_Thread, SetThreadName)
{
    ThreadHandle handle = CreateThread([]() {
        SetCurrentThreadName("TestThread");
        Sleep(10);
    });
    
    JoinThread(handle);
    
    SUCCEED();
}
