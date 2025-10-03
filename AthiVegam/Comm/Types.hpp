#pragma once

#include "Core/Types.hpp"
#include <string_view>

namespace Engine::Comm {

/// @brief Delivery mode for channel messages
enum class DeliveryMode
{
    /// @brief Immediate synchronous delivery (game-critical events)
    Sync,
    
    /// @brief Job-backed asynchronous delivery (default)
    /// @note Phase 1: Stubbed as synchronous. Phase 2: Will use Job System
    Async,
    
    /// @brief Frame-scoped buffered delivery (drained explicitly)
    Buffered
};

/// @brief Event category for routing
enum class EventCategory
{
    Gameplay,
    UI,
    System
};

/// @brief Channel identifier (hashed topic string)
using ChannelId = u64;

/// @brief Subscriber identifier
using SubscriberId = u64;

} // namespace Engine::Comm

