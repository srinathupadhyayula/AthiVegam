#pragma once
#include <cstdint>
#include <span>
#include <string_view>

namespace Engine::Jobs {

using JobHandle = std::uint64_t;

enum class JobPriority : std::uint32_t {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3
};

struct ResourceKey { std::uint64_t id; };

struct JobDesc {
    std::string_view name{};
    JobPriority priority{JobPriority::Normal};
    std::span<const ResourceKey> reads{};
    std::span<const ResourceKey> writes{};
};

} // namespace Engine::Jobs
