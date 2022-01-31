#pragma once

#include "AthiVegam/App.h"
#include "AthiVegam/Engine.h"
#include "AthiVegam/Graphics/Mesh.h"
#include "AthiVegam/Graphics/Shader.h"

namespace Parugu
{
	class Editor : public AthiVegam::App
	{
	  public:
		virtual ~Editor();
		void Initialize() override;
		void Shutdown() override;
		void Update() override;
		void Render() override;

	  private:
		std::shared_ptr<AthiVegam::Graphics::Mesh> m_mesh;
		std::shared_ptr<AthiVegam::Graphics::Shader>
		    m_shader;
		float xOffset = 0.f;
		float yOffset = 0.f;
		float keySpeed = 0.001f;
	};
} // namespace Parugu

extern std::unique_ptr<AthiVegam::App> CreateApp();
