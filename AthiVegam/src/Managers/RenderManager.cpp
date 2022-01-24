#include "AthiVegam/Managers/RenderManager.h"
#include "AthiVegam/Graphics/Helpers.h"

#include "AthiVegam/Log.h"

#include "glad/glad.h"

namespace AthiVegam::Managers
{
	void RenderManager::Initialize()
	{
		VEGAM_INFO("OpenGl Info\t:\n\t\t\t  Vendor\t:\t{}\n\t\t\t  Renderer\t:\t{}\n\t\t\t  Version\t:\t{}"
			, glGetString(GL_VENDOR)
			, glGetString(GL_RENDERER)
			, glGetString(GL_VERSION));

		glEnable(GL_DEPTH_TEST); VEGAM_CHECK_GL_ERROR;
		glDepthFunc(GL_LEQUAL); VEGAM_CHECK_GL_ERROR;

		glEnable(GL_BLEND); VEGAM_CHECK_GL_ERROR;
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); VEGAM_CHECK_GL_ERROR;

		// Cornflower blue
		SetClearColor(static_cast<float>(0x64) / static_cast<float>(0xFF)
			, static_cast<float>(0x95) / static_cast<float>(0xFF)
			, static_cast<float>(0xED) / static_cast<float>(0xFF)
			, 1.f);
	}

	void RenderManager::Shutdown()
	{
		while (!m_renderCommands.empty())
		{
			m_renderCommands.pop();
		}
	}

	void RenderManager::Clear()
	{
		VEGAM_ASSERT(m_renderCommands.empty(), "Unflushed Render Commands in Queue!");
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); VEGAM_CHECK_GL_ERROR;
	}

	void RenderManager::SetClearColor(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a); VEGAM_CHECK_GL_ERROR;
	}

	void RenderManager::SetWireframeMode(bool enabled)
	{
		glPolygonMode(GL_FRONT_AND_BACK, enabled?GL_LINE:GL_FILL); VEGAM_CHECK_GL_ERROR;
	}

	void RenderManager::Submit(std::unique_ptr<Graphics::RenderCommands::RenderCommand> renderCommand)
	{
		m_renderCommands.push(std::move(renderCommand));
	}

	void RenderManager::Flush()
	{
		while (!m_renderCommands.empty())
		{
			auto rc = std::move(m_renderCommands.front());
			m_renderCommands.pop();

			rc->Execute();
		}
	}
}