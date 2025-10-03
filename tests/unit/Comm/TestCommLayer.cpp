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

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

