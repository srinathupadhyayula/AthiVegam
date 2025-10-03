// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - Hazard Tracker Implementation
// License: MIT

#include "Jobs/HazardTracker.hpp"

namespace Engine::Jobs
{

bool HazardTracker::CanExecute(const std::vector<ResourceKey>& reads,
                                const std::vector<ResourceKey>& writes)
{
    std::lock_guard<std::mutex> lock(_mutex);

    // Check write conflicts
    for (const auto& key : writes)
    {
        auto it = _resources.find(key);
        if (it != _resources.end())
        {
            // Conflict if resource is being read or written
            if (it->second.readCount > 0 || it->second.writing)
            {
                return false;
            }
        }
    }

    // Check read conflicts (only with writes)
    for (const auto& key : reads)
    {
        auto it = _resources.find(key);
        if (it != _resources.end())
        {
            // Conflict if resource is being written
            if (it->second.writing)
            {
                return false;
            }
        }
    }

    return true;
}

void HazardTracker::AcquireResources(const std::vector<ResourceKey>& reads,
                                      const std::vector<ResourceKey>& writes)
{
    std::lock_guard<std::mutex> lock(_mutex);

    // Acquire reads
    for (const auto& key : reads)
    {
        _resources[key].readCount++;
    }

    // Acquire writes
    for (const auto& key : writes)
    {
        _resources[key].writing = true;
    }
}

void HazardTracker::ReleaseResources(const std::vector<ResourceKey>& reads,
                                      const std::vector<ResourceKey>& writes)
{
    std::lock_guard<std::mutex> lock(_mutex);

    // Release reads
    for (const auto& key : reads)
    {
        auto it = _resources.find(key);
        if (it != _resources.end())
        {
            if (it->second.readCount > 0)
            {
                it->second.readCount--;
            }

            // Remove entry if no longer in use
            if (it->second.readCount == 0 && !it->second.writing)
            {
                _resources.erase(it);
            }
        }
    }

    // Release writes
    for (const auto& key : writes)
    {
        auto it = _resources.find(key);
        if (it != _resources.end())
        {
            it->second.writing = false;

            // Remove entry if no longer in use
            if (it->second.readCount == 0 && !it->second.writing)
            {
                _resources.erase(it);
            }
        }
    }
}

} // namespace Engine::Jobs

