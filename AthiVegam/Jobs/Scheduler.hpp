#pragma once
#include <functional>
#include <atomic>
#include "Jobs/Types.hpp"

namespace Engine::Jobs {

class Scheduler {
public:
    static Scheduler& Instance();

    // Initialize/Shutdown worker infrastructure (stubbed for now)
    void Initialize(unsigned /*workers*/ = 0) noexcept { initialized_.store(true, std::memory_order_release); }
    void Shutdown() noexcept { initialized_.store(false, std::memory_order_release); }
    bool IsInitialized() const noexcept { return initialized_.load(std::memory_order_acquire); }

    JobHandle Submit(const JobDesc& desc, std::function<void()> fn);
    void Wait(JobHandle /*h*/) noexcept { /* synchronous stub: work completes in Submit */ }

    template <typename Fn>
    void ParallelFor(std::size_t begin, std::size_t end, std::size_t grain, Fn fn)
    {
        // Synchronous baseline implementation; will be parallelized in subsequent commits
        if (grain == 0) grain = 1;
        for (std::size_t i = begin; i < end; i += grain) {
            const std::size_t chunkEnd = (i + grain < end) ? (i + grain) : end;
            for (std::size_t j = i; j < chunkEnd; ++j) {
                fn(j);
            }
        }
    }

private:
    std::atomic<bool> initialized_{false};
    std::atomic<JobHandle> next_handle_{1};
};

} // namespace Engine::Jobs
