#pragma once

#include <memory>

namespace AthiVegam::Graphics
{
	class Mesh;
	class Shader;
	class Framebuffer;

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
			RenderMesh(std::weak_ptr<Mesh> mesh,
			           std::weak_ptr<Shader> shader);

			virtual void Execute() override;

		  private:
			std::weak_ptr<Mesh> m_mesh;
			std::weak_ptr<Shader> m_shader;
		};

		class PushFramebuffer : public RenderCommand
		{

		  public:
			PushFramebuffer(
			    std::weak_ptr<Framebuffer> framebuffer)
			    : m_framebuffer(framebuffer)
			{
			}

			virtual void Execute() override;

		  private:
			std::weak_ptr<Framebuffer> m_framebuffer;
		};

		class PopFramebuffer : public RenderCommand
		{

		  public:
			PopFramebuffer() {}

			virtual void Execute() override;
		};
	} // namespace RenderCommands
} // namespace AthiVegam::Graphics
