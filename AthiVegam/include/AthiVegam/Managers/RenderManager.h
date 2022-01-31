#pragma once

#include "AthiVegam/Graphics/RenderCommands.h"

#include <memory>
#include <queue>

namespace AthiVegam::Managers
{
	class RenderManager
	{
	  public:
		RenderManager() = default;
		~RenderManager() = default;

		void Initialize();
		void Shutdown();

		void Clear();
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
		std::queue<std::unique_ptr<
		    Graphics::RenderCommands::RenderCommand>>
		    m_renderCommands;
	};
} // namespace AthiVegam::Managers
