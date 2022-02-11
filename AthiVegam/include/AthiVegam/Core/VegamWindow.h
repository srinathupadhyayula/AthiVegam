#pragma once

#include "ImGuiWindow.h"

#include <string>

struct SDL_Window;
using SDL_GLContext = void*;

namespace AthiVegam::Core
{
	struct WindowProperties
	{
		std::string title;
		int x, y, width, height;
		int wMin, hMin;
		int flags;
		float clearColorR, clearColorG, clearColorB;

		WindowProperties();
	};
	class VegamWindow
	{
	  public:
		VegamWindow();
		~VegamWindow();

		bool Create(const WindowProperties& props);
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
