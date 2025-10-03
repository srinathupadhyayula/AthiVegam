// Copyright (c) 2025 Srinath Upadhyayula
// AthiVegam Engine - Application Entry Point
// License: MIT

#pragma once

#include "Core/Application.hpp"
#include <memory>
#include <exception>
#include <iostream>

/// @file EntryPoint.hpp
/// @brief Platform-abstracted entry point for AthiVegam applications
/// @details This header provides the main() function for your application.
///          Include this file ONCE in your main.cpp file, and implement the
///          CreateApplication() function to return your application instance.
///
/// @example
/// @code
/// // main.cpp
/// #include "Core/EntryPoint.hpp"
///
/// class MyApp : public Engine::Application
/// {
/// public:
///     MyApp(const Engine::ApplicationSpecification& spec)
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
/// Engine::Application* Engine::CreateApplication(Engine::ApplicationCommandLineArgs args)
/// {
///     Engine::ApplicationSpecification spec;
///     spec.name = "My Application";
///     spec.commandLineArgs = args;
///     return new MyApp(spec);
/// }
/// @endcode

namespace Engine
{

/// @brief User-defined application factory function
/// @details This function must be implemented by the user to create their application instance.
///          The engine will call this function to obtain the application, then manage its lifecycle.
/// @param args Command-line arguments passed to the application
/// @return Pointer to the user's application instance (ownership transferred to engine)
extern Application* CreateApplication(ApplicationCommandLineArgs args);

} // namespace Engine

/// @brief Main entry point for AthiVegam applications
/// @details This function is provided by the engine and handles:
///          - Calling the user's CreateApplication() factory
///          - Initializing the application
///          - Running the main loop
///          - Shutting down the application
///          - Exception handling
///          - Returning the appropriate exit code
/// @param argc Number of command-line arguments
/// @param argv Array of command-line argument strings
/// @return Exit code (0 for success, non-zero for failure)
int main(int argc, char** argv)
{
    // Prepare command-line arguments
    Engine::ApplicationCommandLineArgs args;
    args.count = argc;
    args.args = argv;

    // Create the application
    Engine::Application* app = nullptr;
    
    try
    {
        // Call user's factory function
        app = Engine::CreateApplication(args);
        
        if (!app)
        {
            std::cerr << "ERROR: CreateApplication() returned nullptr!" << std::endl;
            return 1;
        }

        // Initialize the application
        if (!app->Initialize())
        {
            std::cerr << "ERROR: Application initialization failed!" << std::endl;
            delete app;
            return 1;
        }

        // Run the application
        app->Run();

        // Shutdown the application
        app->Shutdown();

        // Clean up
        delete app;

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "========================================" << std::endl;
        std::cerr << "  FATAL ERROR" << std::endl;
        std::cerr << "========================================" << std::endl;
        std::cerr << "Exception: " << e.what() << std::endl;
        std::cerr << "========================================" << std::endl;

        // Clean up if application was created
        if (app)
        {
            try
            {
                app->Shutdown();
            }
            catch (...)
            {
                // Ignore exceptions during emergency shutdown
            }
            delete app;
        }

        return 1;
    }
    catch (...)
    {
        std::cerr << "========================================" << std::endl;
        std::cerr << "  FATAL ERROR" << std::endl;
        std::cerr << "========================================" << std::endl;
        std::cerr << "Unknown exception occurred!" << std::endl;
        std::cerr << "========================================" << std::endl;

        // Clean up if application was created
        if (app)
        {
            try
            {
                app->Shutdown();
            }
            catch (...)
            {
                // Ignore exceptions during emergency shutdown
            }
            delete app;
        }

        return 1;
    }
}

// Platform-specific entry points can be added here in the future
// For example, WinMain for Windows GUI applications:
//
// #ifdef ATHIVEGAM_PLATFORM_WINDOWS
// #include <Windows.h>
// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
// {
//     return main(__argc, __argv);
// }
// #endif

