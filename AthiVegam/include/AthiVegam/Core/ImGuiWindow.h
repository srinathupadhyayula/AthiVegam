#pragma once


typedef union SDL_Event SDL_Event;

namespace AthiVegam::Core
{
	class ImGuiWindow
	{
	public:
		void Create();
		void Shutdown();

		void HandleSDLEvent(SDL_Event& e);

		void BeginRender();
		void EndRender();
	};
}
