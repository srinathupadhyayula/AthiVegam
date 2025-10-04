// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - Job System Types
// License: MIT

#pragma once

#include "Core/Types.hpp"
#include "Core/Memory/Handle.hpp"
#include <functional>
#include <vector>
#include <string>

namespace Engine::Jobs
{

/// @brief 64-bit resource identifier for hazard tracking
/// @details Used to identify resources (components, buffers, etc.) that jobs access
using ResourceKey = u64;

/// @brief Job priority levels
enum class JobPriority : u8
{
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3
};

/// @brief Job affinity hints for worker thread selection
enum class JobAffinity : u8
{
    Any = 0,        ///< No preference, scheduler decides
    MainThread = 1, ///< Must run on main thread
    WorkerThread = 2 ///< Must run on worker thread (not main)
};

/// @brief Job handle for tracking and waiting on jobs
/// @details 64-bit versioned handle (32-bit index + 32-bit version)
struct JobHandleTag {};
using JobHandle = Handle<JobHandleTag>;

/// @brief Job function signature
using JobFunction = std::function<void()>;

/// @brief Job descriptor with metadata
struct JobDesc
{
    std::string name;                    ///< Debug name for the job
    JobPriority priority = JobPriority::Normal; ///< Execution priority
    JobAffinity affinity = JobAffinity::Any;    ///< Thread affinity hint
    std::vector<ResourceKey> reads;      ///< Resources this job reads from
    std::vector<ResourceKey> writes;     ///< Resources this job writes to
};

/// @brief Job status for tracking completion
enum class JobStatus : u8
{
    Pending,    ///< Job is queued but not yet executing
    Running,    ///< Job is currently executing
    Completed,  ///< Job has finished successfully
    Cancelled   ///< Job was cancelled before execution
};

} // namespace Engine::Jobs

