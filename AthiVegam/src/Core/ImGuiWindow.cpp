#include "AthiVegam/Core/ImGuiWindow.h"

#include "AthiVegam/Engine.h"
#include "SDL2/SDL.h"
#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_opengl3.h"
#include "external/imgui/imgui_impl_sdl.h"

namespace AthiVegam::Core
{
	void
	ImGuiWindow::Create(const ImGuiWindowProperties& props)
	{
		IMGUI_CHECKVERSION();

		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigWindowsMoveFromTitleBarOnly =
		    props.MoveFromTitleBarOnly;
		if (props.IsDockingEnabled)
		{
			io.ConfigFlags |=
			    ImGuiConfigFlags_DockingEnable;
		}
		if (props.IsViewPortEnabled)
		{
			io.ConfigFlags |=
			    ImGuiConfigFlags_ViewportsEnable;
		}

		auto& vegamWindow = Engine::Instance().GetWindow();
		ImGui_ImplSDL2_InitForOpenGL(
		    vegamWindow.GetSDLWindow(),
		    vegamWindow.GetGLContext());
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiWindow::Shutdown()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiWindow::HandleSDLEvent(SDL_Event& e)
	{
		ImGui_ImplSDL2_ProcessEvent(&e);
	}

	bool ImGuiWindow::WantCaptureMouse()
	{
		return ImGui::GetIO().WantCaptureMouse;
	}

	bool ImGuiWindow::WantCaptureKeyboard()
	{
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	void ImGuiWindow::BeginRender()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(
		    Engine::Instance().GetWindow().GetSDLWindow());
		ImGui::NewFrame();
	}

	void ImGuiWindow::EndRender()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(
		    ImGui::GetDrawData());

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags
		    & ImGuiConfigFlags_ViewportsEnable)
		{
			auto& window = Engine::Instance().GetWindow();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			SDL_GL_MakeCurrent(window.GetSDLWindow(),
			                   window.GetGLContext());
		}
	}
} // namespace AthiVegam::Core
