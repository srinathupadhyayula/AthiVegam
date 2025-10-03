#include "Comm/Bus.hpp"
#include "Core/Logger.hpp"
#include <functional>

namespace Engine::Comm {

Bus& Bus::Instance()
{
    static Bus instance;
    return instance;
}

void Bus::Initialize()
{
    if (_initialized)
    {
        Logger::Warn("[Comm] Bus already initialized");
        return;
    }
    
    Logger::Info("[Comm] Initializing message bus");
    
    // Register default logging channel
    ChannelDesc loggingDesc{
        .topic = "system.logging",
        .mode = DeliveryMode::Sync,
        .category = EventCategory::System
    };
    
    auto result = RegisterChannel(loggingDesc);
    if (!result)
    {
        Logger::Error("[Comm] Failed to register logging channel");
    }
    
    _initialized = true;
    Logger::Info("[Comm] Message bus initialized");
}

void Bus::Shutdown()
{
    if (!_initialized)
    {
        return;
    }
    
    Logger::Info("[Comm] Shutting down message bus ({} channels)", _channels.size());
    
    _channels.clear();
    _topicToId.clear();
    _initialized = false;
    
    Logger::Info("[Comm] Message bus shutdown complete");
}

std::expected<ChannelId, BusError> Bus::RegisterChannel(const ChannelDesc& desc)
{
    if (desc.topic.empty())
    {
        Logger::Error("[Comm] Cannot register channel with empty topic");
        return std::unexpected(BusError::InvalidTopic);
    }
    
    ChannelId id = HashTopic(desc.topic);
    
    // Check if channel already exists
    if (_channels.contains(id))
    {
        Logger::Warn("[Comm] Channel already exists: topic='{}'", desc.topic);
        return std::unexpected(BusError::ChannelAlreadyExists);
    }
    
    // Create channel
    auto channel = std::make_unique<Channel>(id, desc);
    _channels[id] = std::move(channel);
    _topicToId[desc.topic] = id;
    
    Logger::Debug("[Comm] Channel registered: topic='{}', id={}", desc.topic, id);
    
    return id;
}

Channel* Bus::GetChannel(ChannelId id)
{
    auto it = _channels.find(id);
    if (it != _channels.end())
    {
        return it->second.get();
    }
    return nullptr;
}

Channel* Bus::GetChannelByTopic(std::string_view topic)
{
    auto it = _topicToId.find(std::string(topic));
    if (it != _topicToId.end())
    {
        return GetChannel(it->second);
    }
    return nullptr;
}

std::expected<void, BusError> Bus::Publish(ChannelId id, const Payload& payload)
{
    Channel* channel = GetChannel(id);
    if (!channel)
    {
        Logger::Error("[Comm] Channel not found: id={}", id);
        return std::unexpected(BusError::ChannelNotFound);
    }
    
    channel->Publish(payload);
    return {};
}

std::expected<void, BusError> Bus::PublishToTopic(std::string_view topic, const Payload& payload)
{
    Channel* channel = GetChannelByTopic(topic);
    if (!channel)
    {
        Logger::Error("[Comm] Channel not found: topic='{}'", topic);
        return std::unexpected(BusError::ChannelNotFound);
    }
    
    channel->Publish(payload);
    return {};
}

std::expected<SubscriberId, BusError> Bus::Subscribe(ChannelId id, SubscriberCallback callback)
{
    Channel* channel = GetChannel(id);
    if (!channel)
    {
        Logger::Error("[Comm] Channel not found: id={}", id);
        return std::unexpected(BusError::ChannelNotFound);
    }
    
    SubscriberId subId = channel->Subscribe(std::move(callback));
    return subId;
}

std::expected<SubscriberId, BusError> Bus::SubscribeToTopic(std::string_view topic, SubscriberCallback callback)
{
    Channel* channel = GetChannelByTopic(topic);
    if (!channel)
    {
        Logger::Error("[Comm] Channel not found: topic='{}'", topic);
        return std::unexpected(BusError::ChannelNotFound);
    }
    
    SubscriberId subId = channel->Subscribe(std::move(callback));
    return subId;
}

void Bus::DrainAll()
{
    for (auto& [id, channel] : _channels)
    {
        if (channel->GetMode() == DeliveryMode::Buffered)
        {
            channel->Drain();
        }
    }
}

ChannelId Bus::HashTopic(std::string_view topic)
{
    // Use std::hash for topic string
    return std::hash<std::string_view>{}(topic);
}

} // namespace Engine::Comm

