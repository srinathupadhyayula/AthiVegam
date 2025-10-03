// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - Application Implementation
// License: MIT

#include "Core/Application.hpp"
#include "Core/Platform/Platform.hpp"
#include "Core/Platform/Time.hpp"
#include "Core/Logger.hpp"
#include <iostream>

namespace Engine
{

Application::Application(const ApplicationSpecification& spec)
    : m_specification(spec)
    , m_running(false)
    , m_initialized(false)
{
}

Application::~Application()
{
    if (m_initialized)
    {
        Shutdown();
    }
}

bool Application::Initialize()
{
    if (m_initialized)
    {
        std::cerr << "Application already initialized!" << std::endl;
        return false;
    }

    std::cout << "========================================" << std::endl;
    std::cout << "  " << m_specification.name << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // ========================================
    // 1. Initialize Platform Subsystem
    // ========================================
    std::cout << "[1/3] Initializing Platform..." << std::endl;
    Platform::Initialize();
    std::cout << "  [OK] Platform initialized" << std::endl;
    std::cout << std::endl;

    // ========================================
    // 2. Initialize Time Subsystem
    // ========================================
    std::cout << "[2/3] Initializing Time..." << std::endl;
    Time::Initialize();
    std::cout << "  [OK] Time initialized" << std::endl;
    std::cout << std::endl;

    // ========================================
    // 3. Initialize Logger Subsystem
    // ========================================
    std::cout << "[3/3] Initializing Logger..." << std::endl;
    
    // Configure logger with sensible defaults
    LoggerConfig logConfig;
    logConfig.consoleLevel = LogLevel::Info;
    logConfig.fileLevel = LogLevel::Debug;
    
    // Create log file path based on application name
    std::string logFileName = m_specification.name;
    // Replace spaces with underscores for filename
    for (char& c : logFileName)
    {
        if (c == ' ')
            c = '_';
    }
    logConfig.logFilePath = "logs/" + logFileName + ".log";
    
    logConfig.enableConsole = true;
    logConfig.enableFile = true;
    logConfig.enableColor = true;
    
    Logger::Initialize(logConfig);
    std::cout << "  [OK] Logger initialized" << std::endl;
    std::cout << std::endl;

    // Log engine startup
    LOG_INFO("========================================");
    LOG_INFO("  {} - Starting", m_specification.name);
    LOG_INFO("========================================");
    LOG_INFO("");

    // Log platform information
    LOG_INFO("[Platform Information]");
    LOG_INFO("  Logical CPU Cores:  {}", Platform::GetLogicalCoreCount());
    LOG_INFO("  Physical CPU Cores: {}", Platform::GetPhysicalCoreCount());
    LOG_INFO("  Page Size:          {} bytes", Platform::GetPageSize());
    LOG_INFO("  Cache Line Size:    {} bytes", Platform::GetCacheLineSize());
    LOG_INFO("");

    // Log command-line arguments if any
    if (m_specification.commandLineArgs.count > 0)
    {
        LOG_INFO("[Command-Line Arguments]");
        for (i32 i = 0; i < m_specification.commandLineArgs.count; ++i)
        {
            LOG_INFO("  [{}]: {}", i, m_specification.commandLineArgs[i]);
        }
        LOG_INFO("");
    }

    m_initialized = true;

    // Call user initialization hook
    // NOTE: Users should initialize Jobs and Comm systems in OnInitialize()
    // Example:
    //   Jobs::Scheduler::Instance().Initialize();
    //   Comm::Bus::Instance().Initialize();
    LOG_INFO("[Application] Calling OnInitialize()...");
    OnInitialize();
    LOG_INFO("[Application] OnInitialize() complete");
    LOG_INFO("");

    return true;
}

void Application::Run()
{
    if (!m_initialized)
    {
        std::cerr << "Application not initialized! Call Initialize() first." << std::endl;
        return;
    }

    m_running = true;

    LOG_INFO("[Application] Starting main loop...");
    LOG_INFO("");

    // Call user update hook
    // NOTE: In Phase 1, this will be expanded to a proper game loop with delta time
    OnUpdate();

    LOG_INFO("");
    LOG_INFO("[Application] Main loop complete");

    m_running = false;
}

void Application::Shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    LOG_INFO("");
    LOG_INFO("[Application] Shutting down...");
    LOG_INFO("");

    // Call user shutdown hook
    LOG_INFO("[Application] Calling OnShutdown()...");
    OnShutdown();
    LOG_INFO("[Application] OnShutdown() complete");
    LOG_INFO("");

    // Call user shutdown hook
    // NOTE: Users should shutdown Jobs and Comm systems in OnShutdown()
    // Example:
    //   Comm::Bus::Instance().Shutdown();
    //   Jobs::Scheduler::Instance().Shutdown();

    // Shutdown subsystems in reverse order
    LOG_INFO("========================================");
    LOG_INFO("  {} - Shutdown", m_specification.name);
    LOG_INFO("========================================");
    LOG_INFO("");

    // Shutdown Logger (must be last so we can log shutdown messages)
    Logger::Shutdown();
    std::cout << "  [OK] Logger shutdown" << std::endl;

    // Shutdown Platform
    Platform::Shutdown();
    std::cout << "  [OK] Platform shutdown" << std::endl;

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "  All subsystems shutdown successfully" << std::endl;
    std::cout << "========================================" << std::endl;

    m_initialized = false;
}

} // namespace Engine

