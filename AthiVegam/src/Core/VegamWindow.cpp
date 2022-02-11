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
	WindowProperties::WindowProperties()
	{
		title = "VegamApp";
		x = SDL_WINDOWPOS_CENTERED;
		y = SDL_WINDOWPOS_CENTERED;
		width = 1920;
		height = 1080;
		wMin = 320;
		hMin = 180;
		flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
		clearColorR = static_cast<float>(0x64)
		              / static_cast<float>(0xFF);
		clearColorG = static_cast<float>(0x95)
		              / static_cast<float>(0xFF);
		clearColorB = static_cast<float>(0xED)
		              / static_cast<float>(0xFF);
	}

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

	bool VegamWindow::Create(const WindowProperties& props)
	{
		m_sdlWindow = SDL_CreateWindow(
		    props.title.c_str(), props.x, props.y,
		    props.width, props.height, props.flags);

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

		SDL_SetWindowMinimumSize(m_sdlWindow, props.wMin,
		                         props.hMin);

		m_glContext = SDL_GL_CreateContext(m_sdlWindow);

		if (!m_glContext)
		{
			VEGAM_ERROR("Error creating OpelGl context: {}",
			            SDL_GetError());
			return false;
		}

		gladLoadGLLoader(SDL_GL_GetProcAddress);

		Engine::Instance().GetRenderManager().SetClearColor(
		    props.clearColorR, props.clearColorG,
		    props.clearColorB, 1.0f);

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
		if (!m_imguiWindow.WantCaptureMouse())
		{
			Input::Mouse::Update();
		}

		if (!m_imguiWindow.WantCaptureKeyboard())
		{
			Input::Keyboard::Update();
		}
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
		Engine::Instance().GetApp().ImGuiRender();
		m_imguiWindow.EndRender();

		SDL_GL_SwapWindow(m_sdlWindow);
	}

} // namespace AthiVegam::Core
