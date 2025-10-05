#include "Core/Platform/Threading.hpp"
#include <windows.h>
#include <processthreadsapi.h>
#include <string>
#include <spdlog/spdlog.h>
#include <stdexcept>


namespace Engine::Threading {

namespace {
    // Thread function wrapper
    struct ThreadContext
    {
        ThreadFunction function;
    };

    DWORD WINAPI ThreadProc(LPVOID param)
    {
        auto* context = static_cast<ThreadContext*>(param);
        try
        {
            context->function();
        }
        catch (...)
        {
            // Avoid throwing across threads; optionally log
            spdlog::error("Unhandled exception in thread function");
            delete context;
            return static_cast<DWORD>(-1);
        }
        delete context;
        return 0;
    }
}

ThreadHandle CreateThread(ThreadFunction fn, ThreadPriority priority)
{
    auto* context = new ThreadContext{std::move(fn)};

    HANDLE handle = ::CreateThread(
        nullptr,
        0,
        ThreadProc,
        context,
        0,
        nullptr
    );

    if (handle == nullptr)
    {
        delete context;
        spdlog::error("Failed to create thread");
        return nullptr;
    }

    // Set thread priority
    int winPriority = THREAD_PRIORITY_NORMAL;
    switch (priority)
    {
        case ThreadPriority::Low:
            winPriority = THREAD_PRIORITY_BELOW_NORMAL;
            break;
        case ThreadPriority::Normal:
            winPriority = THREAD_PRIORITY_NORMAL;
            break;
        case ThreadPriority::High:
            winPriority = THREAD_PRIORITY_ABOVE_NORMAL;
            break;
        case ThreadPriority::Critical:
            winPriority = THREAD_PRIORITY_HIGHEST;
            break;
    }

    SetThreadPriority(handle, winPriority);

    return reinterpret_cast<void*>(handle);
}

void JoinThread(ThreadHandle handle)
{
    if (handle != nullptr)
    {
        WaitForSingleObject(reinterpret_cast<HANDLE>(handle), INFINITE);
        CloseHandle(reinterpret_cast<HANDLE>(handle));
    }
}

void DetachThread(ThreadHandle handle)
{
    if (handle != nullptr)
    {
        CloseHandle(reinterpret_cast<HANDLE>(handle));
    }
}

u64 GetCurrentThreadId()
{
    return static_cast<u64>(::GetCurrentThreadId());
}

void SetCurrentThreadName(const char* name)
{
    if (name == nullptr)
    {
        return;
    }

    // Convert to wide string
    int wideSize = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
    if (wideSize == 0)
    {
        return;
    }

    std::wstring wideName(wideSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, name, -1, wideName.data(), wideSize);

    // Set thread description (Windows 10 1607+)
    // Use typedef to avoid compile-time dependency
    typedef HRESULT(WINAPI* SetThreadDescriptionFunc)(HANDLE, PCWSTR);

    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    if (kernel32)
    {
        auto setThreadDesc = reinterpret_cast<SetThreadDescriptionFunc>(
            GetProcAddress(kernel32, "SetThreadDescription"));

        if (setThreadDesc)
        {
            setThreadDesc(GetCurrentThread(), wideName.c_str());
        }
    }
}

void Sleep(u32 milliseconds)
{
    ::Sleep(milliseconds);
}

void YieldThread()
{
    SwitchToThread();
}

// Mutex implementation

Mutex::Mutex()
{
    _handle = new CRITICAL_SECTION();
#if _WIN32_WINNT >= 0x0600
    if (!InitializeCriticalSectionEx(static_cast<CRITICAL_SECTION*>(_handle), 0, 0))
    {
        spdlog::error("InitializeCriticalSectionEx failed: {}", GetLastError());
        delete static_cast<CRITICAL_SECTION*>(_handle);
        _handle = nullptr;
        throw std::runtime_error("Failed to initialize Mutex");
    }
#else
    __try {
        InitializeCriticalSection(static_cast<CRITICAL_SECTION*>(_handle));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        spdlog::error("InitializeCriticalSection raised exception");
        delete static_cast<CRITICAL_SECTION*>(_handle);
        _handle = nullptr;
        throw;
    }
#endif
}

Mutex::~Mutex()
{
    if (_handle != nullptr)
    {
        DeleteCriticalSection(static_cast<CRITICAL_SECTION*>(_handle));
        delete static_cast<CRITICAL_SECTION*>(_handle);
    }
}

void Mutex::Lock()
{
    EnterCriticalSection(static_cast<CRITICAL_SECTION*>(_handle));
}

bool Mutex::TryLock()
{
    return TryEnterCriticalSection(static_cast<CRITICAL_SECTION*>(_handle)) != 0;
}

void Mutex::Unlock()
{
    LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(_handle));
}

// ConditionVariable implementation

ConditionVariable::ConditionVariable()
{
    _handle = new CONDITION_VARIABLE();
    InitializeConditionVariable(static_cast<CONDITION_VARIABLE*>(_handle));
}

ConditionVariable::~ConditionVariable()
{
    if (_handle != nullptr)
    {
        // No cleanup needed for CONDITION_VARIABLE
        delete static_cast<CONDITION_VARIABLE*>(_handle);
    }
}

void ConditionVariable::Wait(Mutex& mutex)
{
    SleepConditionVariableCS(
        static_cast<CONDITION_VARIABLE*>(_handle),
        static_cast<CRITICAL_SECTION*>(mutex._handle),
        INFINITE
    );
}

bool ConditionVariable::WaitFor(Mutex& mutex, u32 timeoutMs)
{
    BOOL result = SleepConditionVariableCS(
        static_cast<CONDITION_VARIABLE*>(_handle),
        static_cast<CRITICAL_SECTION*>(mutex._handle),
        timeoutMs
    );

    return result != 0;
}

void ConditionVariable::NotifyOne()
{
    WakeConditionVariable(static_cast<CONDITION_VARIABLE*>(_handle));
}

void ConditionVariable::NotifyAll()
{
    WakeAllConditionVariable(static_cast<CONDITION_VARIABLE*>(_handle));
}

// RWLock implementation (uses SRWLOCK)

RWLock::RWLock()
{
    _handle = new SRWLOCK();
    InitializeSRWLock(static_cast<SRWLOCK*>(_handle));
}

RWLock::~RWLock()
{
    if (_handle != nullptr)
    {
        // SRWLOCK does not require explicit destruction
        delete static_cast<SRWLOCK*>(_handle);
        _handle = nullptr;
    }
}

void RWLock::LockShared()
{
    AcquireSRWLockShared(static_cast<SRWLOCK*>(_handle));
}

bool RWLock::TryLockShared()
{
    return TryAcquireSRWLockShared(static_cast<SRWLOCK*>(_handle)) != 0;
}

void RWLock::UnlockShared()
{
    ReleaseSRWLockShared(static_cast<SRWLOCK*>(_handle));
}

void RWLock::LockExclusive()
{
    AcquireSRWLockExclusive(static_cast<SRWLOCK*>(_handle));
}

bool RWLock::TryLockExclusive()
{
    return TryAcquireSRWLockExclusive(static_cast<SRWLOCK*>(_handle)) != 0;
}

void RWLock::UnlockExclusive()
{
    ReleaseSRWLockExclusive(static_cast<SRWLOCK*>(_handle));
}

// Semaphore implementation

Semaphore::Semaphore(u32 initialCount)
{
    // Max count: large positive value
    constexpr LONG kMaxCount = 0x7fffffff;
    _handle = CreateSemaphoreW(nullptr, static_cast<LONG>(initialCount), kMaxCount, nullptr);

    if (_handle == nullptr)
    {
        spdlog::error("CreateSemaphoreW failed: {}", GetLastError());
        throw std::runtime_error("Failed to create Semaphore");
    }
}

Semaphore::~Semaphore()
{
    if (_handle != nullptr)
    {
        CloseHandle(static_cast<HANDLE>(_handle));
        _handle = nullptr;
    }
}

void Semaphore::Wait()
{
    WaitForSingleObject(static_cast<HANDLE>(_handle), INFINITE);
}

bool Semaphore::TryWait()
{
    DWORD res = WaitForSingleObject(static_cast<HANDLE>(_handle), 0);
    return res == WAIT_OBJECT_0;
}

void Semaphore::Signal()
{
    if (!ReleaseSemaphore(static_cast<HANDLE>(_handle), 1, nullptr))
    {
        spdlog::error("ReleaseSemaphore failed: {}", GetLastError());
    }
}

} // namespace Engine::Threading
