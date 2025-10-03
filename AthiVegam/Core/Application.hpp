// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - Application Base Class
// License: MIT

#pragma once

#include "Core/Types.hpp"
#include <string>
#include <vector>

namespace Engine
{

/// @brief Command-line arguments passed to the application
struct ApplicationCommandLineArgs
{
    i32 count = 0;
    char** args = nullptr;

    /// @brief Get argument at index
    /// @param index Index of the argument (0 = program name)
    /// @return Argument string or empty string if index out of bounds
    [[nodiscard]] const char* operator[](i32 index) const
    {
        return (index >= 0 && index < count) ? args[index] : "";
    }
};

/// @brief Specification for creating an application
struct ApplicationSpecification
{
    std::string name = "AthiVegam Application";
    std::string workingDirectory;
    ApplicationCommandLineArgs commandLineArgs;
};

/// @brief Base class for all AthiVegam applications
/// @details This class provides the foundation for creating applications using the AthiVegam engine.
///          Users should inherit from this class and override the virtual lifecycle methods:
///          - OnInitialize(): Called after engine subsystems are initialized
///          - OnUpdate(): Called each frame (currently once, will be expanded in Phase 1)
///          - OnShutdown(): Called before engine subsystems are shut down
///
/// @example
/// @code
/// class MyApp : public Engine::Application
/// {
/// public:
///     MyApp(const ApplicationSpecification& spec)
///         : Application(spec)
///     {
///     }
///
///     void OnInitialize() override
///     {
///         LOG_INFO("MyApp initialized!");
///     }
///
///     void OnUpdate() override
///     {
///         LOG_INFO("MyApp running...");
///     }
///
///     void OnShutdown() override
///     {
///         LOG_INFO("MyApp shutting down...");
///     }
/// };
///
/// // In your main.cpp:
/// Engine::Application* Engine::CreateApplication(ApplicationCommandLineArgs args)
/// {
///     ApplicationSpecification spec;
///     spec.name = "My Application";
///     spec.commandLineArgs = args;
///     return new MyApp(spec);
/// }
/// @endcode
class Application
{
public:
    /// @brief Construct an application with the given specification
    /// @param spec Application specification containing name, working directory, and command-line args
    explicit Application(const ApplicationSpecification& spec);

    /// @brief Virtual destructor
    virtual ~Application();

    /// @brief Initialize the application and all engine subsystems
    /// @details This method initializes Phase 0 subsystems in the correct order:
    ///          1. Platform subsystem
    ///          2. Time subsystem
    ///          3. Logger subsystem
    ///          Then calls the user's OnInitialize() hook.
    /// @return true if initialization succeeded, false otherwise
    bool Initialize();

    /// @brief Run the application main loop
    /// @details Currently runs a single iteration. Will be expanded to a proper game loop in Phase 1.
    ///          Calls OnUpdate() for user code execution.
    void Run();

    /// @brief Shutdown the application and all engine subsystems
    /// @details Calls the user's OnShutdown() hook, then shuts down Phase 0 subsystems
    ///          in reverse order of initialization.
    void Shutdown();

    /// @brief Get the application specification
    /// @return Reference to the application specification
    [[nodiscard]] const ApplicationSpecification& GetSpecification() const { return m_specification; }

    /// @brief Get the application name
    /// @return Application name string
    [[nodiscard]] const std::string& GetName() const { return m_specification.name; }

    /// @brief Check if the application is running
    /// @return true if running, false otherwise
    [[nodiscard]] bool IsRunning() const { return m_running; }

protected:
    /// @brief User-defined initialization hook
    /// @details Called after engine subsystems are initialized.
    ///          Override this to perform application-specific initialization.
    virtual void OnInitialize() {}

    /// @brief User-defined update hook
    /// @details Called each frame during the main loop.
    ///          Override this to implement application logic.
    virtual void OnUpdate() {}

    /// @brief User-defined shutdown hook
    /// @details Called before engine subsystems are shut down.
    ///          Override this to perform application-specific cleanup.
    virtual void OnShutdown() {}

private:
    ApplicationSpecification m_specification;
    bool m_running = false;
    bool m_initialized = false;
};

} // namespace Engine

