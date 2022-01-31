#include "AthiVegam/Core/VegamWindow.h"

#include "AthiVegam/Engine.h"
#include "AthiVegam/Graphics/Helpers.h"
#include "AthiVegam/Input/Controller.h"
#include "AthiVegam/Input/Keyboard.h"
#include "AthiVegam/Input/Mouse.h"
#include "AthiVegam/Log.h"
#include "SDL2/SDL.h"
#include "external/imgui/imgui.h"
#include "glad/glad.h"

namespace AthiVegam::Core
{
	VegamWindow::VegamWindow()
	    : m_sdlWindow(nullptr), m_glContext(nullptr)
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
		m_sdlWindow = SDL_CreateWindow(
		    "AthiVegamGame", SDL_WINDOWPOS_CENTERED,
		    SDL_WINDOWPOS_CENTERED, 800, 600,
		    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		if (!m_sdlWindow)
		{
			VEGAM_ERROR("Error creating window: {}",
			            SDL_GetError());
			return false;
		}

		VEGAM_INFO("OpenGl Context Created!");

#ifdef AV_PLATFORM_MAC
		SDL_GL_SetAttribute(
		    SDL_GL_CONTEXT_FLAGS,
		    SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif // AV_PLATFORM_MAC

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
		                    SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,
		                    4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,
		                    1);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		SDL_SetWindowSize(m_sdlWindow, 800, 600);

		m_glContext = SDL_GL_CreateContext(m_sdlWindow);

		if (!m_glContext)
		{
			VEGAM_ERROR("Error creating OpelGl context: {}",
			            SDL_GetError());
			return false;
		}

		gladLoadGLLoader(SDL_GL_GetProcAddress);

		m_imguiWindow.Create();

		return true;
	}

	void VegamWindow::Shutdown()
	{
		SDL_DestroyWindow(m_sdlWindow);
		SDL_GL_DeleteContext(m_glContext);
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
			case SDL_CONTROLLERDEVICEADDED:
				Input::Controller::OnControllerConnected(
				    e.cdevice);
				break;
			case SDL_CONTROLLERDEVICEREMOVED:
				Input::Controller::OnControllerDisconnected(
				    e.cdevice);
				break;
			default:
				break;
			}

			m_imguiWindow.HandleSDLEvent(e);
		}

		// Update Input
		Input::Mouse::Update();
		Input::Keyboard::Update();
		Input::Controller::Update();
	}

	void VegamWindow::GetSize(int& w, int& h)
	{
		SDL_GetWindowSize(m_sdlWindow, &w, &h);
	}

	void VegamWindow::BeginRender()
	{
		Engine::Instance().GetRenderManager().Clear();
	}

	void VegamWindow::EndRender()
	{
		m_imguiWindow.BeginRender();
		ImGui::ShowDemoWindow();
		m_imguiWindow.EndRender();
		SDL_GL_SwapWindow(m_sdlWindow);
	}
} // namespace AthiVegam::Core
