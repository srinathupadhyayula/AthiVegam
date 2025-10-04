#include "Jobs/Scheduler.hpp"

namespace Engine::Jobs {

Scheduler& Scheduler::Instance()
{
    static Scheduler inst;
    return inst;
}

JobHandle Scheduler::Submit(const JobDesc& /*desc*/, std::function<void()> fn)
{
    const JobHandle h = next_handle_.fetch_add(1, std::memory_order_relaxed);

    // Synchronous baseline: execute immediately (will become queued/parallel)
    try {
        fn();
    } catch (...) {
        // Silently catch exceptions in baseline implementation
    }

    return h;
}

} // namespace Engine::Jobs
