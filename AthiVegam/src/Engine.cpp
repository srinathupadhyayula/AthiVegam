#include "Engine.h"
#include "sdl2/SDL.h"

#include <iostream>

namespace AthiVegam 
{
    Engine::Engine() 
        : m_isRunning(false) 
    { 
        GetInfo(); 
    }

    Engine::~Engine() 
    { 
        Shutdown();
    }

    bool Engine::Initialize() 
    {
        bool ret = false;
        
        if (SDL_Init(SDL_INIT_EVERYTHING)) 
        {
            std::cout << "Error initializing SDL2: " << SDL_GetError() << std::endl;
            ret = false;
        } 
        else 
        {
            SDL_version version;
            SDL_VERSION(&version);
            std::cout << "SDL " << (int32_t)version.major << "."
                      << (int32_t)version.minor << "." << (int32_t)version.patch
                      << std::endl;

            if (m_window.Create()) 
            {
                ret = true;
            }
        }

        m_isRunning = ret;

        if (!ret) 
        {
             std::cout << "Engine failed to initialize!" << std::endl;
            Shutdown();
        }

        return ret;
    }

    void Engine::Shutdown() 
    {
        m_window.Shutdown();
        SDL_Quit();
    }

    void Engine::Run() 
    {
        if (Initialize()) 
        {
            while (m_isRunning) 
            {
                m_window.PumpEvents();
            }
        }

        Shutdown();
    }

    void Engine::Quit() 
    {
        m_isRunning = false; 
    }

    void Engine::GetInfo() 
    {
    #ifdef AV_CONFIG_DEBUG
        std::cout << "Configuration : DEBUG" << std::endl;
    #endif //  AV_CONFIG_DEBUG
    #ifdef AV_CONFIG_RELEASE
        std::cout << "Configuration : RELEASE" << std::endl;
    #endif // AV_CONFIG_RELEASE
    #ifdef AV_PLATFORM_WINDOWS
        std::cout << "Platform : WINDOWS" << std::endl;
    #endif // AV_PLATFORM_WINDOWS
    #ifdef AV_PLATFORM_LINUX
        std::cout << "Platform : LINUX" << std::endl;
    #endif // AV_PLATFORM_LINUX
    #ifdef AV_PLATFORM_MAC
        std::cout << "Platform : MAC" << std::endl;
    #endif // AV_PLATFORM_MAC
    }
} // namespace AthiVegam
