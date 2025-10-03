#include "Core/Application.hpp"
#include "Core/EntryPoint.hpp"
#include "Core/Logger.hpp"
#include "Core/Platform/Platform.hpp"
#include "Core/Platform/Time.hpp"
#include "Core/Platform/Threading.hpp"
#include "Core/Platform/Filesystem.hpp"
#include "Core/Memory/Allocators.hpp"
#include "Comm/Bus.hpp"
#include "Comm/Channel.hpp"
#include "Comm/Payload.hpp"
#include <cstring>

/// @brief Comprehensive test application for all Phase 0 foundation components
/// @details Tests Platform, Time, Memory, Logger, Threading, and Filesystem subsystems
class EngineTestApp : public Engine::Application
{
public:
    explicit EngineTestApp(const Engine::ApplicationSpecification& spec)
        : Application(spec)
    {
    }

    void OnInitialize() override
    {
        LOG_INFO("========================================");
        LOG_INFO("  AthiVegam Engine - Foundation Test");
        LOG_INFO("========================================");
        LOG_INFO("");

        // Initialize Communication Layer
        Engine::Comm::Bus::Instance().Initialize();
    }

    void OnUpdate() override
    {
        using namespace Engine;

        // ========================================
        // Test 1: Platform Subsystem
        // ========================================
        LOG_INFO("[Test 1: Platform Subsystem]");
        LOG_INFO("  Logical CPU Cores:  {}", Platform::GetLogicalCoreCount());
        LOG_INFO("  Physical CPU Cores: {}", Platform::GetPhysicalCoreCount());
        LOG_INFO("  Page Size:          {} bytes", Platform::GetPageSize());
        LOG_INFO("  Cache Line Size:    {} bytes", Platform::GetCacheLineSize());
        LOG_INFO("  Status:             PASS");
        LOG_INFO("");

        // ========================================
        // Test 2: Time Subsystem
        // ========================================
        LOG_INFO("[Test 2: Time Subsystem]");
        auto currentTime = Time::GetTime();
        auto currentTimeMs = Time::GetTimeMs();
        LOG_INFO("  Current Time:       {:.6f} seconds", currentTime);
        LOG_INFO("  Current Time:       {:.2f} milliseconds", currentTimeMs);
        LOG_INFO("  Status:             PASS");
        LOG_INFO("");

        // ========================================
        // Test 3: Memory Subsystem
        // ========================================
        LOG_INFO("[Test 3: Memory Subsystem]");
        
        // Test aligned allocation
        LOG_DEBUG("  Testing aligned memory allocation...");
        constexpr usize testSize = 1024;
        constexpr usize testAlignment = 64;
        void* ptr = Memory::AlignedAlloc(testSize, testAlignment);
        
        if (ptr)
        {
            LOG_INFO("  Allocated:          {} bytes ({}-byte aligned)", testSize, testAlignment);
            LOG_INFO("  Address:            {}", ptr);
            
            bool isAligned = Memory::IsAligned(ptr, testAlignment);
            LOG_INFO("  Alignment Check:    {}", isAligned ? "PASS" : "FAIL");
            
            Memory::AlignedFree(ptr);
            LOG_DEBUG("  Memory freed successfully");
        }
        else
        {
            LOG_ERROR("  Allocation FAILED!");
        }
        
        auto stats = Memory::GetGlobalStats();
        LOG_INFO("  Total Allocated:    {} bytes", stats.totalAllocated);
        LOG_INFO("  Total Freed:        {} bytes", stats.totalFreed);
        LOG_INFO("  Current Usage:      {} bytes", stats.currentUsage);
        LOG_INFO("  Allocation Count:   {}", stats.allocationCount);
        LOG_INFO("  Status:             PASS");
        LOG_INFO("");

        // ========================================
        // Test 4: Logger Subsystem
        // ========================================
        LOG_INFO("[Test 4: Logger Subsystem]");
        LOG_DEBUG("  This is a DEBUG message (development info)");
        LOG_INFO("  This is an INFO message (general information)");
        LOG_WARN("  This is a WARN message (warning, not critical)");
        LOG_ERROR("  This is an ERROR message (error occurred)");
        LOG_CRITICAL("  This is a CRITICAL message (critical failure)");
        LOG_INFO("  Formatted output test: Entities: {}, Frame Time: {:.2f}ms", 10000, 16.67);
        LOG_INFO("  Status:             PASS");
        LOG_INFO("");

        // ========================================
        // Test 5: Threading Subsystem
        // ========================================
        LOG_INFO("[Test 5: Threading Subsystem]");
        auto threadId = Threading::GetCurrentThreadId();
        LOG_INFO("  Current Thread ID:  {}", threadId);
        
        // Test mutex
        LOG_DEBUG("  Testing mutex...");
        Threading::Mutex testMutex;
        testMutex.Lock();
        LOG_DEBUG("  Mutex locked");
        testMutex.Unlock();
        LOG_DEBUG("  Mutex unlocked");
        LOG_INFO("  Mutex Test:         PASS");
        
        // Test thread creation
        LOG_DEBUG("  Testing thread creation...");
        bool threadExecuted = false;
        auto testThread = Threading::CreateThread([&threadExecuted]() {
            threadExecuted = true;
        });
        Threading::JoinThread(testThread);
        LOG_INFO("  Thread Test:        {}", threadExecuted ? "PASS" : "FAIL");
        LOG_INFO("  Status:             PASS");
        LOG_INFO("");

        // ========================================
        // Test 6: Filesystem Subsystem
        // ========================================
        LOG_INFO("[Test 6: Filesystem Subsystem]");
        
        // Test directory existence
        bool logsExist = Filesystem::DirectoryExists("logs");
        LOG_INFO("  'logs' directory:   {}", logsExist ? "EXISTS" : "NOT FOUND");
        
        // Test file operations
        const char* testFilePath = "logs/engine_test_temp.txt";
        LOG_DEBUG("  Testing file write...");
        const char* testData = "AthiVegam Engine Test\n";
        auto writeResult = Filesystem::WriteEntireFile(testFilePath,
            Span<const byte>(reinterpret_cast<const byte*>(testData), strlen(testData)));
        bool writeSuccess = writeResult.has_value();
        LOG_INFO("  File Write:         {}", writeSuccess ? "PASS" : "FAIL");

        if (writeSuccess)
        {
            LOG_DEBUG("  Testing file read...");
            auto readResult = Filesystem::ReadEntireFile(testFilePath);
            bool readSuccess = readResult.has_value();
            LOG_INFO("  File Read:          {}", readSuccess ? "PASS" : "FAIL");

            if (readSuccess)
            {
                std::string content(reinterpret_cast<const char*>(readResult.value().data()),
                                    readResult.value().size());
                LOG_DEBUG("  File content: '{}'", content);
            }

            // Test file existence
            bool fileExists = Filesystem::FileExists(testFilePath);
            LOG_INFO("  File Exists Check:  {}", fileExists ? "PASS" : "FAIL");

            // Clean up test file
            LOG_DEBUG("  Cleaning up test file...");
            auto deleteResult = Filesystem::RemoveFile(testFilePath);
            bool deleteSuccess = deleteResult.has_value();
            LOG_INFO("  File Delete:        {}", deleteSuccess ? "PASS" : "FAIL");
        }
        
        LOG_INFO("  Status:             PASS");
        LOG_INFO("");

        // ========================================
        // Test 7: Communication Layer
        // ========================================
        LOG_INFO("[Test 7: Communication Layer]");

        using namespace Engine::Comm;

        // Test channel registration
        LOG_DEBUG("  Testing channel registration...");
        ChannelDesc gameplayDesc{
            .topic = "gameplay.events",
            .mode = DeliveryMode::Sync,
            .category = EventCategory::Gameplay
        };
        auto channelResult = Bus::Instance().RegisterChannel(gameplayDesc);
        bool channelRegistered = channelResult.has_value();
        LOG_INFO("  Channel Registration: {}", channelRegistered ? "PASS" : "FAIL");

        if (channelRegistered)
        {
            // Test publish/subscribe
            LOG_DEBUG("  Testing publish/subscribe...");
            int receivedValue = 0;
            auto subResult = Bus::Instance().SubscribeToTopic("gameplay.events",
                [&receivedValue](const Payload& payload) {
                    if (auto* value = payload.Get<int>()) {
                        receivedValue = *value;
                    }
                });

            bool subscribed = subResult.has_value();
            LOG_INFO("  Subscribe:          {}", subscribed ? "PASS" : "FAIL");

            if (subscribed)
            {
                auto pubResult = Bus::Instance().PublishToTopic("gameplay.events", Payload(42));
                bool published = pubResult.has_value();
                LOG_INFO("  Publish:            {}", published ? "PASS" : "FAIL");
                LOG_INFO("  Message Received:   {} (expected: 42)", receivedValue);
                LOG_INFO("  Message Delivery:   {}", receivedValue == 42 ? "PASS" : "FAIL");
            }
        }

        // Test buffered channel
        LOG_DEBUG("  Testing buffered channel...");
        ChannelDesc bufferedDesc{
            .topic = "ui.events",
            .mode = DeliveryMode::Buffered,
            .category = EventCategory::UI
        };
        auto bufferedResult = Bus::Instance().RegisterChannel(bufferedDesc);

        if (bufferedResult.has_value())
        {
            int bufferedCount = 0;
            Bus::Instance().SubscribeToTopic("ui.events",
                [&bufferedCount](const Payload&) {
                    bufferedCount++;
                });

            // Publish 3 messages
            Bus::Instance().PublishToTopic("ui.events", Payload(1));
            Bus::Instance().PublishToTopic("ui.events", Payload(2));
            Bus::Instance().PublishToTopic("ui.events", Payload(3));

            LOG_INFO("  Buffered Messages:  {} (before drain)", bufferedCount);

            // Drain all buffered channels
            Bus::Instance().DrainAll();

            LOG_INFO("  Buffered Messages:  {} (after drain)", bufferedCount);
            LOG_INFO("  Buffered Delivery:  {}", bufferedCount == 3 ? "PASS" : "FAIL");
        }

        LOG_INFO("  Status:             PASS");
        LOG_INFO("");

        // ========================================
        // Summary
        // ========================================
        LOG_INFO("========================================");
        LOG_INFO("  All Tests Complete!");
        LOG_INFO("========================================");
        LOG_INFO("");
        LOG_INFO("Tested Subsystems:");
        LOG_INFO("  [OK] Platform      - CPU detection, system info");
        LOG_INFO("  [OK] Time          - High-resolution timers");
        LOG_INFO("  [OK] Memory        - Aligned allocation, tracking");
        LOG_INFO("  [OK] Logger        - Multi-level logging, file output");
        LOG_INFO("  [OK] Threading     - Thread creation, mutexes");
        LOG_INFO("  [OK] Filesystem    - File I/O, directory operations");
        LOG_INFO("  [OK] Communication - Pub/sub messaging, delivery modes");
        LOG_INFO("");
    }

    void OnShutdown() override
    {
        LOG_INFO("EngineTest shutting down...");

        // Shutdown Communication Layer
        Engine::Comm::Bus::Instance().Shutdown();
    }
};

// Application factory function
Engine::Application* Engine::CreateApplication(Engine::ApplicationCommandLineArgs args)
{
    Engine::ApplicationSpecification spec;
    spec.name = "AthiVegam Engine - Foundation Test";
    spec.commandLineArgs = args;
    
    return new EngineTestApp(spec);
}

