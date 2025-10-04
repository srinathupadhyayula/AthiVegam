#pragma once
#include <cstdint>

namespace Engine::Jobs {

// Minimal placeholder; future Windows Fiber-based cooperative waiting
class FiberContext {
public:
    void* native{nullptr};
};

} // namespace Engine::Jobs
