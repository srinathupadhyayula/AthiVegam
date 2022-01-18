#pragma once
#include <memory>

struct SDL_Window;

namespace AthiVegam::Core
{
	class VegamWindow
	{
	public:
		VegamWindow();
		~VegamWindow();

		bool Create();
		void Shutdown();
		void PumpEvents();

	private:
		SDL_Window* m_sdlWindow;
	};
}