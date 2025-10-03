#include "Comm/Channel.hpp"
#include "Core/Logger.hpp"
#include "Core/Memory/FrameArena.hpp"
#include <algorithm>

namespace Engine::Comm {

Channel::Channel(ChannelId id, const ChannelDesc& desc)
    : _id(id)
    , _desc(desc)
{
    // Initialize frame arena for Buffered mode
    if (_desc.mode == DeliveryMode::Buffered)
    {
        // Allocate 64KB arena for buffered messages
        _frameArena = std::make_unique<Memory::FrameArena>(65536);
    }
    
    Logger::Debug("[Comm] Channel created: topic='{}', mode={}", 
        _desc.topic, static_cast<int>(_desc.mode));
}

Channel::~Channel()
{
    Logger::Debug("[Comm] Channel destroyed: topic='{}'", _desc.topic);
}

void Channel::Publish(const Payload& payload)
{
    switch (_desc.mode)
    {
        case DeliveryMode::Sync:
            // Immediate synchronous delivery
            InvokeSubscribers(payload);
            break;
            
        case DeliveryMode::Async:
            // TODO: Phase 2 - Implement job-backed async delivery
            // For now, deliver synchronously
            Logger::Trace("[Comm] Async mode stubbed as sync for topic '{}'", _desc.topic);
            InvokeSubscribers(payload);
            break;
            
        case DeliveryMode::Buffered:
        {
            // Queue message for frame-scoped drainage
            std::lock_guard<std::mutex> lock(_queueMutex);
            if (_desc.maxQueueSize > 0 && _messageQueue.size() >= _desc.maxQueueSize)
            {
                Logger::Warn("[Comm] Message queue full for topic '{}', dropping message",
                    _desc.topic);
                return;
            }

            _messageQueue.push_back(BufferedMessage{payload});
            break;
        }
    }
}

SubscriberId Channel::Subscribe(SubscriberCallback callback)
{
    if (!callback)
    {
        Logger::Error("[Comm] Attempted to subscribe with null callback to topic '{}'",
            _desc.topic);
        return 0;
    }

    std::lock_guard<std::mutex> lock(_subscribersMutex);
    SubscriberId id = _nextSubscriberId++;
    _subscribers.push_back(Subscriber{id, std::move(callback)});

    Logger::Debug("[Comm] Subscriber {} added to topic '{}'", id, _desc.topic);

    return id;
}

bool Channel::Unsubscribe(SubscriberId id)
{
    std::lock_guard<std::mutex> lock(_subscribersMutex);
    auto it = std::find_if(_subscribers.begin(), _subscribers.end(),
        [id](const Subscriber& sub) { return sub.id == id; });

    if (it != _subscribers.end())
    {
        _subscribers.erase(it);
        Logger::Debug("[Comm] Subscriber {} removed from topic '{}'", id, _desc.topic);
        return true;
    }

    Logger::Warn("[Comm] Subscriber {} not found in topic '{}'", id, _desc.topic);
    return false;
}

void Channel::Drain()
{
    if (_desc.mode != DeliveryMode::Buffered)
    {
        Logger::Warn("[Comm] Drain() called on non-buffered channel '{}'", _desc.topic);
        return;
    }

    // Copy messages to process (prevents holding lock during callback invocation)
    std::vector<BufferedMessage> messagesToProcess;
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        if (_messageQueue.empty())
        {
            return;
        }

        Logger::Trace("[Comm] Draining {} messages from topic '{}'",
            _messageQueue.size(), _desc.topic);

        messagesToProcess.swap(_messageQueue);
    }

    // Process all queued messages (without holding lock)
    for (const auto& msg : messagesToProcess)
    {
        InvokeSubscribers(msg.payload);
    }

    // Reset arena after processing
    if (_frameArena)
    {
        _frameArena->Reset();
    }
}

void Channel::InvokeSubscribers(const Payload& payload)
{
    // Copy subscribers to prevent iterator invalidation if a callback
    // modifies the subscriber list (subscribe/unsubscribe)
    std::vector<Subscriber> subscribersCopy;
    {
        std::lock_guard<std::mutex> lock(_subscribersMutex);
        if (_subscribers.empty())
        {
            return;
        }
        subscribersCopy = _subscribers;
    }

    // Invoke all subscribers from the copied list (without holding the lock)
    for (const auto& subscriber : subscribersCopy)
    {
        try
        {
            subscriber.callback(payload);
        }
        catch (const std::exception& e)
        {
            Logger::Error("[Comm] Exception in subscriber {} for topic '{}': {}",
                subscriber.id, _desc.topic, e.what());
        }
        catch (...)
        {
            Logger::Error("[Comm] Unknown exception in subscriber {} for topic '{}'",
                subscriber.id, _desc.topic);
        }
    }
}

} // namespace Engine::Comm

