#pragma once

#include "Core/VegamWindow.h"
#include "Managers/LogManager.h"
#include "Managers/RenderManager.h"

#include <memory>

namespace AthiVegam 
{
	class Engine 
	{
	public:
		inline static auto& Instance()
		{
			// Magic Static pattern for Singleton instance
			static Engine instance;
			return instance;
		}

		~Engine();

		// delete Copy & Move constructors/operators
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(Engine&&) = delete;

		// Engine Methods
		void Run();
		void Quit();

		// Getters for Managers
		inline Managers::RenderManager& GetRenderManager() { return m_renderManager; }
		inline Core::VegamWindow& GetWindow() { return m_window; }

	private:
		// Singleton for now
		Engine();

		[[nodiscard]] bool Initialize();
		void Shutdown();

		void GetInfo();

	private:
		bool m_isRunning;
		bool m_isInitialized;

		Core::VegamWindow m_window;

		// Managers
		Managers::LogManager m_logManager;
		Managers::RenderManager m_renderManager;
	};
} // namespace AthiVegam
