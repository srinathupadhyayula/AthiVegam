#pragma once

#include "Core/VegamWindow.h"
#include "Managers/LogManager.h"

#include <memory>

namespace AthiVegam 
{
	class Engine 
	{
	public:
		~Engine();
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(Engine&&) = delete;

		void Run();
		void Quit();

		static auto& Instance()
		{
			static Engine instance;
			return instance;
		}

	private:
		// Singleton for now
		Engine();

		[[nodiscard]] bool Initialize();
		void Shutdown();

		void GetInfo();

	private:
		Core::VegamWindow m_window;
		Managers::LogManager m_logManager;
		bool m_isRunning;
		bool m_isInitialized;

	};
} // namespace AthiVegam
