#pragma once
#include <cstdint>
#include <span>
#include "Jobs/Types.hpp"

namespace Engine::Jobs {

// Placeholder for hazard tracking; will be implemented with read/write set checks
class HazardTracker {
public:
    bool Allows(const JobDesc& /*desc*/) const noexcept { return true; }
};

} // namespace Engine::Jobs
