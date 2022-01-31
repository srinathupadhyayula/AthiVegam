#include "Parugu/Editor.h"

#include "AthiVegam/Log.h"
#include "AthiVegam/Main.h"
#include "Athivegam/Input/Controller.h"
#include "Athivegam/Input/Keyboard.h"
#include "Athivegam/Input/Mouse.h"

using namespace AthiVegam;

std::unique_ptr<App> CreateApp()
{
	return std::make_unique<Parugu::Editor>();
}

namespace Parugu
{
	Editor::~Editor()
	{
		m_shader.reset();
		m_mesh.reset();
	}

	void Editor::Initialize()
	{
		VEGAM_WARN("Editor Initialized!");
		// Tesh Mesh
		float verts[]{0.5f,  0.5f,  0.0f,  0.5f,
		              -0.5f, 0.0f,  -0.5f, -0.5f,
		              0.0f,  -0.5f, 0.5f,  0.0f};
		uint32_t elements[]{0, 1, 3, 1, 2, 3};
		// auto mesh =
		// std::make_shared<Graphics::Mesh>(&verts[0], 3,
		// 3);
		m_mesh = std::make_shared<Graphics::Mesh>(
		    &verts[0], 4, 3, &elements[0], 6);

		// Test Shader
		const char* vertShader = R"(
                    #version 410 core
                    layout (location = 0) in vec3 position;
                    out vec3 vpos;
                    uniform vec2 offset = vec2(0.5f);
                    void main()
                    {
                        vpos = position + vec3(offset, 0);
                        gl_Position = vec4(position, 1.0f);
                    }
                )";
		const char* fragShader = R"(
                    #version 410 core
                    out vec4 outColor;
                    in vec3 vpos;
                    uniform vec3 color = vec3(0.0f);
                    void main()
                    {
                        //outColor = vec4(color, 1.0);
                        outColor = vec4(vpos, 1.0);
                    }
                )";
		m_shader = std::make_shared<Graphics::Shader>(
		    vertShader, fragShader);
		m_shader->SetUniformFloat3("color", 1.f, 0.f, 0.f);
	}

	void Editor::Shutdown()
	{
		VEGAM_WARN("Editor Shutdown!");
	}

	void Editor::Update() {}

	void Editor::Render()
	{
		auto rc = std::make_unique<
		    Graphics::RenderCommands::RenderMesh>(m_mesh,
		                                          m_shader);
		Engine::Instance().GetRenderManager().Submit(
		    std::move(rc));
		Engine::Instance().GetRenderManager().Flush();
	}
} // namespace Parugu
