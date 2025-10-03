#include <gtest/gtest.h>
#include "Comm/Bus.hpp"
#include "Comm/Channel.hpp"
#include "Comm/Payload.hpp"
#include "Core/Logger.hpp"
#include <thread>
#include <atomic>
#include <chrono>

using namespace Engine;
using namespace Engine::Comm;

class CommLayerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize logger for tests
        Logger::Initialize();
        
        // Initialize bus
        Bus::Instance().Initialize();
    }
    
    void TearDown() override
    {
        // Shutdown bus
        Bus::Instance().Shutdown();
        
        // Shutdown logger
        Logger::Shutdown();
    }
};

// ============================================================================
// Payload Tests
// ============================================================================

TEST_F(CommLayerTest, Payload_IntConstruction)
{
    Payload payload(42);
    
    ASSERT_TRUE(payload.Is<int>());
    ASSERT_FALSE(payload.IsEmpty());
    
    auto* value = payload.Get<int>();
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 42);
}

TEST_F(CommLayerTest, Payload_StringConstruction)
{
    Payload payload("Hello, World!");
    
    ASSERT_TRUE(payload.Is<std::string>());
    ASSERT_FALSE(payload.IsEmpty());
    
    auto* value = payload.Get<std::string>();
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, "Hello, World!");
}

TEST_F(CommLayerTest, Payload_TypeMismatch)
{
    Payload payload(42);
    
    auto* floatValue = payload.Get<float>();
    EXPECT_EQ(floatValue, nullptr);
}

TEST_F(CommLayerTest, Payload_Empty)
{
    Payload payload;
    
    EXPECT_TRUE(payload.IsEmpty());
    EXPECT_EQ(payload.Get<int>(), nullptr);
}

// ============================================================================
// Channel Tests - Sync Mode
// ============================================================================

TEST_F(CommLayerTest, Channel_SyncDelivery_SingleSubscriber)
{
    ChannelDesc desc{.topic = "test.sync", .mode = DeliveryMode::Sync};
    Channel channel(1, desc);
    
    int receivedValue = 0;
    channel.Subscribe([&receivedValue](const Payload& payload) {
        if (auto* value = payload.Get<int>()) {
            receivedValue = *value;
        }
    });
    
    channel.Publish(Payload(42));
    
    EXPECT_EQ(receivedValue, 42);
}

TEST_F(CommLayerTest, Channel_SyncDelivery_MultipleSubscribers)
{
    ChannelDesc desc{.topic = "test.sync.multi", .mode = DeliveryMode::Sync};
    Channel channel(1, desc);
    
    int count = 0;
    channel.Subscribe([&count](const Payload&) { count++; });
    channel.Subscribe([&count](const Payload&) { count++; });
    channel.Subscribe([&count](const Payload&) { count++; });
    
    channel.Publish(Payload(1));
    
    EXPECT_EQ(count, 3);
}

TEST_F(CommLayerTest, Channel_SyncDelivery_MessageOrdering)
{
    ChannelDesc desc{.topic = "test.sync.order", .mode = DeliveryMode::Sync};
    Channel channel(1, desc);
    
    std::vector<int> received;
    channel.Subscribe([&received](const Payload& payload) {
        if (auto* value = payload.Get<int>()) {
            received.push_back(*value);
        }
    });
    
    channel.Publish(Payload(1));
    channel.Publish(Payload(2));
    channel.Publish(Payload(3));
    
    ASSERT_EQ(received.size(), 3);
    EXPECT_EQ(received[0], 1);
    EXPECT_EQ(received[1], 2);
    EXPECT_EQ(received[2], 3);
}

// ============================================================================
// Channel Tests - Buffered Mode
// ============================================================================

TEST_F(CommLayerTest, Channel_BufferedDelivery_QueueAndDrain)
{
    ChannelDesc desc{.topic = "test.buffered", .mode = DeliveryMode::Buffered};
    Channel channel(1, desc);
    
    int receivedValue = 0;
    channel.Subscribe([&receivedValue](const Payload& payload) {
        if (auto* value = payload.Get<int>()) {
            receivedValue = *value;
        }
    });
    
    // Publish messages (should be queued)
    channel.Publish(Payload(10));
    channel.Publish(Payload(20));
    channel.Publish(Payload(30));
    
    // Messages not delivered yet
    EXPECT_EQ(receivedValue, 0);
    
    // Drain queue
    channel.Drain();
    
    // Last message should be received
    EXPECT_EQ(receivedValue, 30);
}

TEST_F(CommLayerTest, Channel_BufferedDelivery_MultipleMessages)
{
    ChannelDesc desc{.topic = "test.buffered.multi", .mode = DeliveryMode::Buffered};
    Channel channel(1, desc);
    
    std::vector<int> received;
    channel.Subscribe([&received](const Payload& payload) {
        if (auto* value = payload.Get<int>()) {
            received.push_back(*value);
        }
    });
    
    channel.Publish(Payload(1));
    channel.Publish(Payload(2));
    channel.Publish(Payload(3));
    
    channel.Drain();
    
    ASSERT_EQ(received.size(), 3);
    EXPECT_EQ(received[0], 1);
    EXPECT_EQ(received[1], 2);
    EXPECT_EQ(received[2], 3);
}

// ============================================================================
// Channel Tests - Async Mode (Stubbed)
// ============================================================================

TEST_F(CommLayerTest, Channel_AsyncDelivery_Stubbed)
{
    ChannelDesc desc{.topic = "test.async", .mode = DeliveryMode::Async};
    Channel channel(1, desc);
    
    int receivedValue = 0;
    channel.Subscribe([&receivedValue](const Payload& payload) {
        if (auto* value = payload.Get<int>()) {
            receivedValue = *value;
        }
    });
    
    // Async mode is stubbed as sync in Phase 1
    channel.Publish(Payload(42));
    
    // Should be delivered immediately (stubbed behavior)
    EXPECT_EQ(receivedValue, 42);
}

// ============================================================================
// Channel Tests - Subscription Management
// ============================================================================

TEST_F(CommLayerTest, Channel_Unsubscribe)
{
    ChannelDesc desc{.topic = "test.unsub", .mode = DeliveryMode::Sync};
    Channel channel(1, desc);
    
    int count = 0;
    auto subId = channel.Subscribe([&count](const Payload&) { count++; });
    
    channel.Publish(Payload(1));
    EXPECT_EQ(count, 1);
    
    bool unsubscribed = channel.Unsubscribe(subId);
    EXPECT_TRUE(unsubscribed);
    
    channel.Publish(Payload(2));
    EXPECT_EQ(count, 1); // Should not increment
}

TEST_F(CommLayerTest, Channel_UnsubscribeInvalid)
{
    ChannelDesc desc{.topic = "test.unsub.invalid", .mode = DeliveryMode::Sync};
    Channel channel(1, desc);
    
    bool unsubscribed = channel.Unsubscribe(999);
    EXPECT_FALSE(unsubscribed);
}

// ============================================================================
// Bus Tests
// ============================================================================

TEST_F(CommLayerTest, Bus_RegisterChannel)
{
    ChannelDesc desc{.topic = "test.bus.register", .mode = DeliveryMode::Sync};
    
    auto result = Bus::Instance().RegisterChannel(desc);
    ASSERT_TRUE(result.has_value());
    
    ChannelId id = result.value();
    EXPECT_NE(id, 0);
}

TEST_F(CommLayerTest, Bus_RegisterDuplicateChannel)
{
    ChannelDesc desc{.topic = "test.bus.duplicate", .mode = DeliveryMode::Sync};
    
    auto result1 = Bus::Instance().RegisterChannel(desc);
    ASSERT_TRUE(result1.has_value());
    
    auto result2 = Bus::Instance().RegisterChannel(desc);
    ASSERT_FALSE(result2.has_value());
    EXPECT_EQ(result2.error(), BusError::ChannelAlreadyExists);
}

TEST_F(CommLayerTest, Bus_GetChannel)
{
    ChannelDesc desc{.topic = "test.bus.get", .mode = DeliveryMode::Sync};
    
    auto result = Bus::Instance().RegisterChannel(desc);
    ASSERT_TRUE(result.has_value());
    
    Channel* channel = Bus::Instance().GetChannel(result.value());
    ASSERT_NE(channel, nullptr);
    EXPECT_EQ(channel->GetTopic(), "test.bus.get");
}

TEST_F(CommLayerTest, Bus_GetChannelByTopic)
{
    ChannelDesc desc{.topic = "test.bus.get.topic", .mode = DeliveryMode::Sync};
    
    auto result = Bus::Instance().RegisterChannel(desc);
    ASSERT_TRUE(result.has_value());
    
    Channel* channel = Bus::Instance().GetChannelByTopic("test.bus.get.topic");
    ASSERT_NE(channel, nullptr);
    EXPECT_EQ(channel->GetId(), result.value());
}

TEST_F(CommLayerTest, Bus_PublishToTopic)
{
    ChannelDesc desc{.topic = "test.bus.publish", .mode = DeliveryMode::Sync};
    
    auto result = Bus::Instance().RegisterChannel(desc);
    ASSERT_TRUE(result.has_value());
    
    int receivedValue = 0;
    auto subResult = Bus::Instance().SubscribeToTopic("test.bus.publish", 
        [&receivedValue](const Payload& payload) {
            if (auto* value = payload.Get<int>()) {
                receivedValue = *value;
            }
        });
    ASSERT_TRUE(subResult.has_value());
    
    auto pubResult = Bus::Instance().PublishToTopic("test.bus.publish", Payload(42));
    ASSERT_TRUE(pubResult.has_value());
    
    EXPECT_EQ(receivedValue, 42);
}

TEST_F(CommLayerTest, Bus_DrainAll)
{
    // Register buffered channel
    ChannelDesc desc{.topic = "test.bus.drain", .mode = DeliveryMode::Buffered};
    auto result = Bus::Instance().RegisterChannel(desc);
    ASSERT_TRUE(result.has_value());
    
    int receivedValue = 0;
    auto subResult = Bus::Instance().SubscribeToTopic("test.bus.drain",
        [&receivedValue](const Payload& payload) {
            if (auto* value = payload.Get<int>()) {
                receivedValue = *value;
            }
        });
    ASSERT_TRUE(subResult.has_value());
    
    // Publish to buffered channel
    Bus::Instance().PublishToTopic("test.bus.drain", Payload(100));
    EXPECT_EQ(receivedValue, 0); // Not delivered yet
    
    // Drain all channels
    Bus::Instance().DrainAll();
    EXPECT_EQ(receivedValue, 100); // Now delivered
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST_F(CommLayerTest, Channel_ConcurrentPublish)
{
    ChannelDesc desc{.topic = "test.concurrent", .mode = DeliveryMode::Sync};
    Channel channel(1, desc);

    std::atomic<int> count{0};
    channel.Subscribe([&count](const Payload&) {
        count++;
    });

    constexpr int numThreads = 4;
    constexpr int messagesPerThread = 100;

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back([&channel]() {
            for (int j = 0; j < messagesPerThread; ++j)
            {
                channel.Publish(Payload(j));
            }
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(count.load(), numThreads * messagesPerThread);
}

TEST_F(CommLayerTest, Channel_ConcurrentSubscribeUnsubscribe)
{
    ChannelDesc desc{.topic = "test.concurrent.subsub", .mode = DeliveryMode::Sync};
    Channel channel(1, desc);

    std::atomic<int> subscribeCount{0};
    std::atomic<int> unsubscribeCount{0};
    std::vector<SubscriberId> subscriberIds;
    std::mutex idsMutex;

    constexpr int numThreads = 4;
    constexpr int operationsPerThread = 50;

    std::vector<std::thread> threads;

    // 2 threads subscribing
    for (int i = 0; i < 2; ++i)
    {
        threads.emplace_back([&]() {
            for (int j = 0; j < operationsPerThread; ++j)
            {
                auto id = channel.Subscribe([](const Payload&) {});
                if (id != 0)
                {
                    subscribeCount++;
                    std::lock_guard<std::mutex> lock(idsMutex);
                    subscriberIds.push_back(id);
                }
            }
        });
    }

    // 2 threads unsubscribing
    for (int i = 0; i < 2; ++i)
    {
        threads.emplace_back([&]() {
            for (int j = 0; j < operationsPerThread; ++j)
            {
                // Wait a bit to let some subscribers be added
                std::this_thread::sleep_for(std::chrono::microseconds(10));

                SubscriberId idToRemove = 0;
                {
                    std::lock_guard<std::mutex> lock(idsMutex);
                    if (!subscriberIds.empty())
                    {
                        idToRemove = subscriberIds.back();
                        subscriberIds.pop_back();
                    }
                }

                if (idToRemove != 0 && channel.Unsubscribe(idToRemove))
                {
                    unsubscribeCount++;
                }
            }
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    // Verify no crashes occurred and counts are reasonable
    EXPECT_GT(subscribeCount.load(), 0);
    EXPECT_GT(unsubscribeCount.load(), 0);

    // Final subscriber count should be subscribeCount - unsubscribeCount
    usize expectedCount = subscribeCount.load() - unsubscribeCount.load();
    EXPECT_EQ(channel.GetSubscriberCount(), expectedCount);
}

// ============================================================================
// Callback Re-entrancy Tests (Iterator Invalidation Fix)
// ============================================================================

TEST_F(CommLayerTest, Channel_CallbackSubscribesDuringInvoke)
{
    ChannelDesc desc{.topic = "test.reentrant.subscribe", .mode = DeliveryMode::Sync};
    Channel channel(1, desc);

    std::atomic<int> callbackCount{0};
    std::atomic<bool> newSubscriberCalled{false};

    // First subscriber that will add another subscriber during callback
    channel.Subscribe([&](const Payload& payload) {
        callbackCount++;

        // Add new subscriber during callback invocation
        channel.Subscribe([&](const Payload&) {
            newSubscriberCalled = true;
        });
    });

    // Initial subscriber count
    EXPECT_EQ(channel.GetSubscriberCount(), 1);

    // Publish message - should trigger re-entrant Subscribe()
    channel.Publish(Payload(1));

    // Verify first callback was invoked
    EXPECT_EQ(callbackCount.load(), 1);

    // Verify new subscriber was added (count should be 2 now)
    EXPECT_EQ(channel.GetSubscriberCount(), 2);

    // Publish another message - new subscriber should be invoked
    channel.Publish(Payload(2));

    EXPECT_TRUE(newSubscriberCalled.load());
    EXPECT_EQ(callbackCount.load(), 2); // First subscriber called again
}

TEST_F(CommLayerTest, Channel_CallbackUnsubscribesSelfDuringInvoke)
{
    ChannelDesc desc{.topic = "test.reentrant.unsubscribe", .mode = DeliveryMode::Sync};
    Channel channel(1, desc);

    std::atomic<int> callbackCount{0};
    SubscriberId selfId = 0;

    // Subscriber that unsubscribes itself during callback
    selfId = channel.Subscribe([&](const Payload& payload) {
        callbackCount++;

        // Unsubscribe self during callback invocation
        bool removed = channel.Unsubscribe(selfId);
        EXPECT_TRUE(removed);
    });

    EXPECT_NE(selfId, 0);
    EXPECT_EQ(channel.GetSubscriberCount(), 1);

    // Publish message - callback should unsubscribe itself
    channel.Publish(Payload(1));

    EXPECT_EQ(callbackCount.load(), 1);
    EXPECT_EQ(channel.GetSubscriberCount(), 0);

    // Publish another message - callback should NOT be invoked
    channel.Publish(Payload(2));

    EXPECT_EQ(callbackCount.load(), 1); // Still 1, not called again
}

// ============================================================================
// Initialization and Shutdown Tests
// ============================================================================

TEST_F(CommLayerTest, Bus_OperationsAfterShutdown)
{
    // Shutdown the bus explicitly
    Bus::Instance().Shutdown();

    // Verify RegisterChannel returns error after shutdown
    ChannelDesc desc{.topic = "test.after.shutdown", .mode = DeliveryMode::Sync};
    auto regResult = Bus::Instance().RegisterChannel(desc);
    EXPECT_FALSE(regResult.has_value());

    // Verify GetChannel returns nullptr after shutdown
    Channel* channel = Bus::Instance().GetChannel(1);
    EXPECT_EQ(channel, nullptr);

    // Verify GetChannelByTopic returns nullptr after shutdown
    channel = Bus::Instance().GetChannelByTopic("test.after.shutdown");
    EXPECT_EQ(channel, nullptr);

    // Verify Publish returns error after shutdown
    auto pubResult = Bus::Instance().PublishToTopic("test.after.shutdown", Payload(42));
    EXPECT_FALSE(pubResult.has_value());

    // Verify DrainAll doesn't crash after shutdown
    Bus::Instance().DrainAll(); // Should return early, no crash

    // Re-initialize for cleanup (TearDown will call Shutdown again, which is safe)
    Bus::Instance().Initialize();
}

// Test without fixture to verify operations before Initialize()
TEST(CommLayerTestNoFixture, Bus_OperationsBeforeInitialize)
{
    // Initialize logger (needed for Bus operations)
    Logger::Initialize();

    // Get Bus instance but DON'T call Initialize()
    Bus& bus = Bus::Instance();

    // Verify RegisterChannel returns error before Initialize()
    ChannelDesc desc{.topic = "test.before.init", .mode = DeliveryMode::Sync};
    auto regResult = bus.RegisterChannel(desc);
    EXPECT_FALSE(regResult.has_value());

    // Verify GetChannel returns nullptr before Initialize()
    Channel* channel = bus.GetChannel(1);
    EXPECT_EQ(channel, nullptr);

    // Verify GetChannelByTopic returns nullptr before Initialize()
    channel = bus.GetChannelByTopic("test.before.init");
    EXPECT_EQ(channel, nullptr);

    // Verify DrainAll doesn't crash before Initialize()
    bus.DrainAll(); // Should return early, no crash

    // Now initialize and verify operations work
    bus.Initialize();

    // After Initialize(), operations should work
    auto regResult2 = bus.RegisterChannel(desc);
    EXPECT_TRUE(regResult2.has_value());

    channel = bus.GetChannel(regResult2.value());
    EXPECT_NE(channel, nullptr);

    // Cleanup
    bus.Shutdown();
    Logger::Shutdown();
}

// ============================================================================
// Error Condition Tests
// ============================================================================

TEST_F(CommLayerTest, Bus_RegisterChannelEmptyTopic)
{
    // Attempt to register channel with empty topic
    ChannelDesc desc{.topic = "", .mode = DeliveryMode::Sync};
    auto result = Bus::Instance().RegisterChannel(desc);

    // Should return error
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), BusError::InvalidTopic);
}

TEST_F(CommLayerTest, Channel_SubscribeNullCallback)
{
    ChannelDesc desc{.topic = "test.null.callback", .mode = DeliveryMode::Sync};
    Channel channel(1, desc);

    // Attempt to subscribe with null callback
    SubscriberId id = channel.Subscribe(nullptr);

    // Should return invalid ID (0)
    EXPECT_EQ(id, 0);

    // Subscriber count should remain 0
    EXPECT_EQ(channel.GetSubscriberCount(), 0);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

