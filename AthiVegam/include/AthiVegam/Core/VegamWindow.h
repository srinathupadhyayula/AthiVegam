#pragma once

#include "ImGuiWindow.h"

#include <memory>
#include <string>

struct SDL_Window;
using SDL_GLContext = void*;

namespace AthiVegam::Graphics
{
	class Framebuffer;
}

namespace AthiVegam::Core
{
	struct WindowProperties
	{
		std::string title;
		int x, y, width, height;
		int wMin, hMin;
		int flags;
		float clearColorR, clearColorG, clearColorB;
		ImGuiWindowProperties imguiProps;

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
		inline Graphics::Framebuffer* GetFramebuffer()
		{
			return m_framebuffer.get();
		}

	  private:
		SDL_Window* m_sdlWindow;
		SDL_GLContext m_glContext;
		ImGuiWindow m_imguiWindow;
		std::shared_ptr<Graphics::Framebuffer>
		    m_framebuffer;
	};
} // namespace AthiVegam::Core
