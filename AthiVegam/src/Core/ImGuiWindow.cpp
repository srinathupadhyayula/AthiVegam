#include "AthiVegam/Core/ImGuiWindow.h"

#include "AthiVegam/Engine.h"
#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_opengl3.h"
#include "external/imgui/imgui_impl_sdl.h"

namespace AthiVegam::Core
{
	void ImGuiWindow::Create()
	{
		IMGUI_CHECKVERSION();

		ImGui::CreateContext();
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
	}
} // namespace AthiVegam::Core
