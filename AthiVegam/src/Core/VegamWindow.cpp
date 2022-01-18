#include "Core/VegamWindow.h"
#include "SDL2/SDL.h"
#include "glad/glad.h"
#include "Engine.h"
#include "Log.h"

namespace AthiVegam::Core
{
	VegamWindow::VegamWindow()
		: m_sdlWindow(nullptr)
		, m_glContext(nullptr)
	{
		
	}

	VegamWindow::~VegamWindow()
	{
		if (m_sdlWindow)
		{
			Shutdown();
		}
	}

	bool VegamWindow::Create()
	{
		m_sdlWindow = SDL_CreateWindow("AthiVegamGame", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

		if (!m_sdlWindow)
		{
			VEGAM_ERROR("Error creating window: {}", SDL_GetError());
			return false;
		}

		VEGAM_INFO("OpenGl Context Created!");

#ifdef AV_PLATFORM_MAC
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif //AV_PLATFORM_MAC

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		SDL_SetWindowSize(m_sdlWindow, 200, 200);

		m_glContext = SDL_GL_CreateContext(m_sdlWindow);

		if (!m_glContext)
		{
			VEGAM_ERROR("Error creatinf OpelGl context: {}", SDL_GetError());
			return false;
		}

		gladLoadGLLoader(SDL_GL_GetProcAddress);

		//TODO: Move to renderer
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Cornflower blue
		glClearColor(static_cast<float>(0x64) / static_cast<float>(0xFF)
			, static_cast<float>(0x95) / static_cast<float>(0xFF)
			, static_cast<float>(0xED) / static_cast<float>(0xFF)
			, 1.f);

		return true;
	}

	void VegamWindow::Shutdown()
	{
		SDL_DestroyWindow(m_sdlWindow);
		m_sdlWindow = nullptr;
	}

	void VegamWindow::PumpEvents()
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				Engine::Instance().Quit();
				break;
			default:
				break;
			}
		}
	}

	void VegamWindow::BeginRender()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void VegamWindow::EndRender()
	{
		SDL_GL_SwapWindow(m_sdlWindow);
	}
}

