#pragma once

#include <memory>

namespace AthiVegam::Graphics
{
	class Mesh;
	class Shader;

	namespace RenderCommands
	{
		class RenderCommand
		{
		public:
			virtual void Execute() = 0;
			virtual ~RenderCommand() = default;
		};

		class RenderMesh : public RenderCommand
		{
		public:
			RenderMesh(std::weak_ptr<Mesh> mesh, std::weak_ptr<Shader> shader);

			virtual void Execute();

		private:
			std::weak_ptr<Mesh> m_mesh;
			std::weak_ptr<Shader> m_shader;
		};
	}
}