#pragma once

typedef union SDL_Event SDL_Event;

namespace AthiVegam::Core
{
	struct ImGuiWindowProperties
	{
		bool MoveFromTitleBarOnly = true;
		bool IsDockingEnabled = false;
		bool IsViewPortEnabled = false;
	};

	class ImGuiWindow
	{
	  public:
		void Create(const ImGuiWindowProperties& props);
		void Shutdown();

		void HandleSDLEvent(SDL_Event& e);

		bool WantCaptureMouse();
		bool WantCaptureKeyboard();

		void BeginRender();
		void EndRender();
	};
} // namespace AthiVegam::Core
