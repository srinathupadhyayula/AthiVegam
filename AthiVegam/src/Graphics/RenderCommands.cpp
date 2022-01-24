#include "AthiVegam/Graphics/RenderCommands.h"
#include "AthiVegam/Graphics/Mesh.h"
#include "AthiVegam/Graphics/Shader.h"
#include "AthiVegam/Graphics/Helpers.h"

#include "AthiVegam/Log.h"

#include "glad/glad.h"

namespace AthiVegam::Graphics::RenderCommands
{
	RenderMesh::RenderMesh(std::weak_ptr<Mesh> mesh, std::weak_ptr<Shader> shader)
		: m_mesh(mesh)
		, m_shader(shader)
	{

	}

	void RenderMesh::Execute()
	{
		std::shared_ptr<Mesh> mesh = m_mesh.lock();
		std::shared_ptr<Shader> shader = m_shader.lock();

		if (mesh && shader)
		{
			mesh->Bind();
			shader->Bind();

			if (mesh->GetElementCount() > 0)
			{
				glDrawElements(GL_TRIANGLES, mesh->GetElementCount(), GL_UNSIGNED_INT, 0);
			}
			else
			{
				glDrawArrays(GL_TRIANGLE_STRIP, 0, mesh->GetVertexCount()); VEGAM_CHECK_GL_ERROR;
			}

			shader->Unbind();
			mesh->Unbind();
		}
		else
		{
			VEGAM_WARN("Attempting to execute RenderMesh with invalid data");
		}

	}
}