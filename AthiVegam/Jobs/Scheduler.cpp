#include "Jobs/Scheduler.hpp"
#include "Core/Logger.hpp"

namespace Engine::Jobs {

Scheduler& Scheduler::Instance()
{
    static Scheduler inst;
    return inst;
}

JobHandle Scheduler::Submit(const JobDesc& desc, std::function<void()> fn)
{
    const JobHandle h = next_handle_.fetch_add(1, std::memory_order_relaxed);

    // Synchronous baseline: execute immediately (will become queued/parallel)
    if (!IsInitialized()) {
        Core::Logger::Warn("[Jobs] Scheduler not initialized; executing job '{}' synchronously", desc.name);
    }

    try {
        fn();
    } catch (const std::exception& e) {
        Core::Logger::Error("[Jobs] Job '{}' threw exception: {}", desc.name, e.what());
    } catch (...) {
        Core::Logger::Error("[Jobs] Job '{}' threw unknown exception", desc.name);
    }

    return h;
}

} // namespace Engine::Jobs
