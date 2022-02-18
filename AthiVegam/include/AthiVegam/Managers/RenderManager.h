#pragma once

#include "AthiVegam/Graphics/RenderCommands.h"

#include <memory>
#include <queue>
#include <stack>

#define VEGAM_SUBMIT_RC(type, ...)                         \
	std::move(std::make_unique<                            \
	          AthiVegam::Graphics::RenderCommands::type>(  \
	    __VA_ARGS__))

namespace AthiVegam::Managers
{
	class RenderManager
	{
		friend class AthiVegam::Graphics::RenderCommands::
		    PushFramebuffer;
		friend class AthiVegam::Graphics::RenderCommands::
		    PopFramebuffer;

	  public:
		RenderManager() = default;
		~RenderManager() = default;

		void Initialize();
		void Shutdown();

		void Clear();
		void SetViewport(int x, int y, int width,
		                 int height);
		void SetClearColor(float r, float g, float b,
		                   float a);
		void SetWireframeMode(bool enabled);

		void Submit(std::unique_ptr<
		            Graphics::RenderCommands::RenderCommand>
		                renderCommand);

		// Execute submitted RenderCommands in the order
		// they were received for now.
		void Flush();

	  private:
		void PushFramebuffer(
		    std::shared_ptr<Graphics::Framebuffer>
		        framebuffer);
		void PopFramebuffer();

	  private:
		std::queue<std::unique_ptr<
		    Graphics::RenderCommands::RenderCommand>>
		    m_renderCommands;

		std::stack<std::shared_ptr<Graphics::Framebuffer>>
		    m_framebuffers;
	};
} // namespace AthiVegam::Managers
