#pragma once

#include "ImGuiWindow.h"

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

		inline SDL_Window* GetSDLWindow()
		{
			return m_sdlWindow;
		}
		inline SDL_GLContext GetGLContext()
		{
			return m_glContext;
		}

	  private:
		SDL_Window* m_sdlWindow;
		SDL_GLContext m_glContext;
		ImGuiWindow m_imguiWindow;
	};
} // namespace AthiVegam::Core
