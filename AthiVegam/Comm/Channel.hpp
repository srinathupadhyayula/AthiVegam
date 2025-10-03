#pragma once

#include "Comm/Types.hpp"
#include "Comm/Payload.hpp"
#include "Core/Types.hpp"
#include <string>
#include <string_view>
#include <functional>
#include <vector>
#include <memory>
#include <mutex>

// Forward declaration
namespace Engine::Memory {
    class FrameArena;
}

namespace Engine::Comm {

/// @brief Channel configuration descriptor
struct ChannelDesc
{
    /// @brief Channel topic (used for identification)
    std::string topic;
    
    /// @brief Delivery mode for messages
    DeliveryMode mode = DeliveryMode::Async;
    
    /// @brief Event category (optional, for event routing)
    EventCategory category = EventCategory::System;
    
    /// @brief Maximum queue size for Buffered mode (0 = unlimited)
    usize maxQueueSize = 0;
};

/// @brief Message subscriber callback
using SubscriberCallback = std::function<void(const Payload&)>;

/// @brief Communication channel for publish/subscribe messaging
/// @details Supports multiple delivery modes: Sync, Async, Buffered.
///          Thread-safe for concurrent publish/subscribe operations.
/// @example
/// @code
/// ChannelDesc desc{.topic = "gameplay.events", .mode = DeliveryMode::Sync};
/// Channel channel(1, desc);
/// 
/// // Subscribe
/// auto id = channel.Subscribe([](const Payload& payload) {
///     if (auto* value = payload.Get<int>()) {
///         // Handle message
///     }
/// });
/// 
/// // Publish
/// channel.Publish(Payload(42));
/// 
/// // Unsubscribe
/// channel.Unsubscribe(id);
/// @endcode
class Channel
{
public:
    /// @brief Construct channel
    /// @param id Channel identifier
    /// @param desc Channel configuration
    Channel(ChannelId id, const ChannelDesc& desc);
    
    /// @brief Destructor
    ~Channel();
    
    // Non-copyable
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;
    
    // Movable
    Channel(Channel&&) noexcept = default;
    Channel& operator=(Channel&&) noexcept = default;
    
    /// @brief Publish message to channel
    /// @param payload Message payload
    void Publish(const Payload& payload);
    
    /// @brief Subscribe to channel messages
    /// @param callback Callback function to invoke on message
    /// @return Subscriber ID for unsubscribing
    SubscriberId Subscribe(SubscriberCallback callback);
    
    /// @brief Unsubscribe from channel
    /// @param id Subscriber ID returned from Subscribe()
    /// @return True if subscriber was found and removed
    bool Unsubscribe(SubscriberId id);
    
    /// @brief Drain buffered messages (Buffered mode only)
    /// @details Processes all queued messages and invokes subscribers
    void Drain();
    
    /// @brief Get channel ID
    /// @return Channel identifier
    ChannelId GetId() const { return _id; }
    
    /// @brief Get channel topic
    /// @return Channel topic string
    const std::string& GetTopic() const { return _desc.topic; }
    
    /// @brief Get delivery mode
    /// @return Delivery mode
    DeliveryMode GetMode() const { return _desc.mode; }
    
    /// @brief Get subscriber count
    /// @return Number of active subscribers
    usize GetSubscriberCount() const { return _subscribers.size(); }

private:
    /// @brief Subscriber entry
    struct Subscriber
    {
        SubscriberId id;
        SubscriberCallback callback;
    };
    
    /// @brief Buffered message entry
    struct BufferedMessage
    {
        Payload payload;
    };
    
    /// @brief Invoke all subscribers with payload
    /// @param payload Message payload
    void InvokeSubscribers(const Payload& payload);
    
    ChannelId _id;
    ChannelDesc _desc;
    std::vector<Subscriber> _subscribers;
    SubscriberId _nextSubscriberId = 1;
    mutable std::mutex _subscribersMutex;  // Protects _subscribers and _nextSubscriberId

    // Buffered mode state
    std::vector<BufferedMessage> _messageQueue;
    std::unique_ptr<Memory::FrameArena> _frameArena;  // For buffered message allocation
};

} // namespace Engine::Comm

