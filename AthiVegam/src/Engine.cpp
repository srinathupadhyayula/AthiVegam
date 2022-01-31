#include "AthiVegam/Engine.h"

#include "AthiVegam/Log.h"
#include "Athivegam/Input/Controller.h"
#include "Athivegam/Input/Keyboard.h"
#include "Athivegam/Input/Mouse.h"
#include "SDL2/SDL.h"

#include <string>

namespace AthiVegam
{
	Engine::Engine()
	    : m_isRunning(false)
	    , m_isInitialized(false)
	    , m_app(nullptr)
	{
	}

	Engine::~Engine() { Shutdown(); }

	bool Engine::Initialize()
	{
		bool ret = false;

		VEGAM_ASSERT(
		    !m_isInitialized,
		    "Attempting to call Engine::Initialize() more "
		    "than once!");
		if (!m_isInitialized)
		{
			GetInfo();

			if (SDL_Init(SDL_INIT_EVERYTHING))
			{
				VEGAM_ERROR("Error initializing SDL2: {}",
				            SDL_GetError());
			}
			else
			{
				SDL_version version;
				SDL_VERSION(&version);
				VEGAM_INFO("SDL {}.{}.{}",
				           (int32_t)version.major,
				           (int32_t)version.minor,
				           (int32_t)version.patch);

				if (m_window.Create())
				{
					// Initialize Managers
					m_renderManager.Initialize();

					// Initialize Input
					Input::Mouse::Initialize();
					Input::Keyboard::Initialize();
					m_app->Initialize();

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
		m_isRunning = false;

		/* Shutdown App */
		m_app->Shutdown();
		m_app.reset();

		/* Shutdown managers */
		m_renderManager.Shutdown();
		m_logManager.Shutdown();

		/* Shutdown SDL */
		m_window.Shutdown();
		SDL_Quit();
	}

	void Engine::Run(std::unique_ptr<App> app)
	{
		m_logManager.Initialize();
		VEGAM_ASSERT(!m_app, "Attempting to call Run when "
		                     "a valid app already exists");
		if (m_app)
		{
			return;
		}

		VEGAM_ASSERT(
		    app,
		    "Attempting to call Run with an invalid app");
		if (!app)
		{
			return;
		}

		m_app = std::move(app);

		if (Initialize())
		{
			/* Core Loop */
			while (m_isRunning)
			{
				Update();
				Render();
			}
		}
	}

	void Engine::Quit() { m_isRunning = false; }

	void Engine::Update()
	{
		m_window.PumpEvents();
		m_app->Update();
	}

	void Engine::Render()
	{
		m_window.BeginRender();
		m_app->Render();
		m_window.EndRender();
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
