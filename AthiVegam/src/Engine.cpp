#include "Engine.h"
#include "Log.h"
#include "sdl2/SDL.h"

namespace AthiVegam 
{
    Engine::Engine() 
        : m_isRunning(false) 
        , m_isInitialized(false)
    { 
        
    }

    Engine::~Engine() 
    { 
        Shutdown();
    }

    bool Engine::Initialize() 
    {
        bool ret = false;

        VEGAM_ASSERT(!m_isInitialized, "Attempting to call Engine::Initialize() more than once!");
        if (!m_isInitialized)
        {
            m_logManager.Initialize();
            GetInfo();

            if (SDL_Init(SDL_INIT_EVERYTHING))
            {
                VEGAM_ERROR("Error initializing SDL2: {}", SDL_GetError());
            }
            else
            {
                SDL_version version;
                SDL_VERSION(&version);
                VEGAM_INFO("SDL {}.{}.{}"
                    , (int32_t)version.major
                    , (int32_t)version.minor
                    , (int32_t)version.patch);

                if (m_window.Create())
                {
                    ret = true;
                }
            }

            m_isRunning = ret;
            m_isInitialized = ret;

            if (!ret)
            {
                VEGAM_ERROR("Engine failed to initialize!");
                Shutdown();
            }
        }

        return ret;
    }

    void Engine::Shutdown() 
    {
        m_isInitialized = false;
        // Shutdown managers
        m_logManager.Shutdown();

        // Shutdown SDL
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
        VEGAM_INFO("AthiVegam v{}.{}", 0, 1);
    #ifdef AV_CONFIG_DEBUG
        VEGAM_DEBUG("Configuration : DEBUG");
    #endif //  AV_CONFIG_DEBUG
    #ifdef AV_CONFIG_RELEASE
        VEGAM_DEBUG("Configuration : RELEASE");
    #endif // AV_CONFIG_RELEASE
    #ifdef AV_PLATFORM_WINDOWS
        VEGAM_WARN("Platform : WINDOWS");
    #endif // AV_PLATFORM_WINDOWS
    #ifdef AV_PLATFORM_LINUX
        VEGAM_WARN"Platform : LINUX");
    #endif // AV_PLATFORM_LINUX
    #ifdef AV_PLATFORM_MAC
        VEGAM_WARN"Platform : MAC");
    #endif // AV_PLATFORM_MAC
    }
} // namespace AthiVegam
