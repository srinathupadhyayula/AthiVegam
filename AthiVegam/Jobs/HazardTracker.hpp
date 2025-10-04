// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - Hazard Tracker
// License: MIT

#pragma once

#include "Jobs/Types.hpp"
#include <unordered_map>
#include <unordered_set>
#include <mutex>

namespace Engine::Jobs
{

/// @brief Tracks resource access hazards to prevent data races
/// @details Maintains read/write sets for active jobs and detects conflicts
///          before job execution. Defers conflicting jobs until resources are available.
class HazardTracker
{
public:
    /// @brief Access mode for a resource
    enum class AccessMode : u8
    {
        Read,  ///< Read-only access
        Write  ///< Read-write access
    };

    /// @brief Check if a job can execute without conflicts
    /// @param reads Resources the job will read
    /// @param writes Resources the job will write
    /// @return true if no conflicts, false if job should be deferred
    bool CanExecute(const std::vector<ResourceKey>& reads,
                    const std::vector<ResourceKey>& writes);

    /// @brief Acquire resources for a job
    /// @param reads Resources to acquire for reading
    /// @param writes Resources to acquire for writing
    void AcquireResources(const std::vector<ResourceKey>& reads,
                          const std::vector<ResourceKey>& writes);

    /// @brief Release resources after job completion
    /// @param reads Resources to release from reading
    /// @param writes Resources to release from writing
    void ReleaseResources(const std::vector<ResourceKey>& reads,
                          const std::vector<ResourceKey>& writes);

private:
    /// @brief Resource access tracking
    struct ResourceAccess
    {
        u32 readCount = 0;  ///< Number of active readers
        bool writing = false; ///< Is being written to
    };

    std::unordered_map<ResourceKey, ResourceAccess> _resources;
    std::mutex _mutex;
};

} // namespace Engine::Jobs

