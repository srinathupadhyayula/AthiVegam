#pragma once
#include <memory>

struct SDL_Window;
using SDL_GLContext = void*;

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

		void GetSize(int& w, int& h);
		void BeginRender();
		void EndRender();

	private:
		SDL_Window* m_sdlWindow;
		SDL_GLContext m_glContext;
	};
}