#pragma once

#include "Comm/Types.hpp"
#include "Comm/Channel.hpp"
#include "Comm/Payload.hpp"
#include "Core/Types.hpp"
#include "Core/Result.hpp"
#include <string_view>
#include <unordered_map>
#include <memory>
#include <functional>

namespace Engine::Comm {

/// @brief Error codes for Bus operations
enum class BusError
{
    ChannelNotFound,
    ChannelAlreadyExists,
    InvalidTopic
};

/// @brief Global message bus for publish/subscribe communication
/// @details Singleton managing typed channels for inter-system messaging.
///          Supports multiple delivery modes and event categories.
/// @example
/// @code
/// // Register channel
/// ChannelDesc desc{.topic = "gameplay.events", .mode = DeliveryMode::Sync};
/// auto channelId = Bus::Instance().RegisterChannel(desc);
/// 
/// // Get channel and subscribe
/// if (auto* channel = Bus::Instance().GetChannel(channelId)) {
///     channel->Subscribe([](const Payload& payload) {
///         // Handle message
///     });
/// }
/// 
/// // Publish message
/// Bus::Instance().Publish(channelId, Payload(42));
/// 
/// // Drain all buffered channels
/// Bus::Instance().DrainAll();
/// @endcode
class Bus
{
public:
    /// @brief Get singleton instance
    /// @return Reference to global Bus instance
    static Bus& Instance();
    
    /// @brief Initialize bus
    /// @details Creates default channels (logging, etc.)
    void Initialize();
    
    /// @brief Shutdown bus
    /// @details Clears all channels and subscribers
    void Shutdown();
    
    /// @brief Register new channel
    /// @param desc Channel configuration
    /// @return Channel ID or error
    std::expected<ChannelId, BusError> RegisterChannel(const ChannelDesc& desc);
    
    /// @brief Get channel by ID
    /// @param id Channel identifier
    /// @return Pointer to channel or nullptr if not found
    Channel* GetChannel(ChannelId id);
    
    /// @brief Get channel by topic
    /// @param topic Channel topic string
    /// @return Pointer to channel or nullptr if not found
    Channel* GetChannelByTopic(std::string_view topic);
    
    /// @brief Publish message to channel
    /// @param id Channel identifier
    /// @param payload Message payload
    /// @return Success or error
    std::expected<void, BusError> Publish(ChannelId id, const Payload& payload);
    
    /// @brief Publish message to channel by topic
    /// @param topic Channel topic string
    /// @param payload Message payload
    /// @return Success or error
    std::expected<void, BusError> PublishToTopic(std::string_view topic, const Payload& payload);
    
    /// @brief Subscribe to channel
    /// @param id Channel identifier
    /// @param callback Subscriber callback
    /// @return Subscriber ID or error
    std::expected<SubscriberId, BusError> Subscribe(ChannelId id, SubscriberCallback callback);
    
    /// @brief Subscribe to channel by topic
    /// @param topic Channel topic string
    /// @param callback Subscriber callback
    /// @return Subscriber ID or error
    std::expected<SubscriberId, BusError> SubscribeToTopic(std::string_view topic, SubscriberCallback callback);
    
    /// @brief Drain all buffered channels
    /// @details Processes all queued messages in Buffered mode channels
    void DrainAll();
    
    /// @brief Get channel count
    /// @return Number of registered channels
    usize GetChannelCount() const { return _channels.size(); }
    
    /// @brief Check if bus is initialized
    /// @return True if initialized
    bool IsInitialized() const { return _initialized; }

private:
    // Singleton pattern
    Bus() = default;
    ~Bus() = default;
    Bus(const Bus&) = delete;
    Bus& operator=(const Bus&) = delete;
    
    /// @brief Hash topic string to channel ID
    /// @param topic Topic string
    /// @return Channel ID
    static ChannelId HashTopic(std::string_view topic);
    
    bool _initialized = false;
    std::unordered_map<ChannelId, std::unique_ptr<Channel>> _channels;
    std::unordered_map<std::string, ChannelId> _topicToId;
};

} // namespace Engine::Comm

