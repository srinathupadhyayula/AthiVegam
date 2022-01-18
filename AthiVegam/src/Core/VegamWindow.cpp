#include "Core/VegamWindow.h"
#include "SDL2/SDL.h"
#include "Engine.h"
#include <iostream>


namespace AthiVegam::Core
{
	VegamWindow::VegamWindow()
		: m_sdlWindow(nullptr)
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
		m_sdlWindow = SDL_CreateWindow("AthiVegamGame", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);

		if (!m_sdlWindow)
		{
			std::cout << "Error creating window: " << SDL_GetError() << std::endl;
			return false;
		}
		
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
}

