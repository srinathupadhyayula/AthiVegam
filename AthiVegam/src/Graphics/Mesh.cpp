#include "AthiVegam/Graphics/Mesh.h"
#include "AthiVegam/Graphics/Helpers.h"

#include "glad/glad.h"

namespace AthiVegam::Graphics
{
	Mesh::Mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions)
		: m_vertexCount(vertexCount)
		, m_elementCount(0)
		, m_ebo(0)
	{
		glGenVertexArrays(1, &m_vao); VEGAM_CHECK_GL_ERROR;
		glBindVertexArray(m_vao); VEGAM_CHECK_GL_ERROR;
		{
			glGenBuffers(1, &m_positionsVbo); VEGAM_CHECK_GL_ERROR;
			glBindBuffer(GL_ARRAY_BUFFER, m_positionsVbo); VEGAM_CHECK_GL_ERROR;
			{
				glBufferData(GL_ARRAY_BUFFER, static_cast<uint64_t>(vertexCount) * dimensions * sizeof(float), vertexArray, GL_STATIC_DRAW); VEGAM_CHECK_GL_ERROR;

				glEnableVertexAttribArray(0); VEGAM_CHECK_GL_ERROR;
				glVertexAttribPointer(0, dimensions, GL_FLOAT, GL_FALSE, 0, 0); VEGAM_CHECK_GL_ERROR;
				glDisableVertexAttribArray(0);  VEGAM_CHECK_GL_ERROR;
			}
			glBindBuffer(GL_ARRAY_BUFFER, 0); VEGAM_CHECK_GL_ERROR;
		}
		glBindVertexArray(0); VEGAM_CHECK_GL_ERROR;
	}

	Mesh::Mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions, uint32_t* elementArray, uint32_t elementCount)
		: Mesh(vertexArray, vertexCount, dimensions)
	{
		m_elementCount = elementCount;
		
		glBindVertexArray(m_vao); VEGAM_CHECK_GL_ERROR;
		{
			glGenBuffers(1, &m_ebo); VEGAM_CHECK_GL_ERROR;
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo); VEGAM_CHECK_GL_ERROR;
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<uint64_t>(elementCount) * sizeof(uint32_t), elementArray, GL_STATIC_DRAW); VEGAM_CHECK_GL_ERROR;
		}
		glBindVertexArray(0); VEGAM_CHECK_GL_ERROR;
	}

	Mesh::~Mesh()
	{
		glDeleteBuffers(1, &m_positionsVbo); VEGAM_CHECK_GL_ERROR;
		if (m_ebo != 0)
		{
			glDeleteBuffers(1, &m_ebo);
		}
		glDeleteVertexArrays(1, &m_vao); VEGAM_CHECK_GL_ERROR;
	}

	void Mesh::Bind()
	{
		glBindVertexArray(m_vao); VEGAM_CHECK_GL_ERROR;
		glEnableVertexAttribArray(0); VEGAM_CHECK_GL_ERROR;
	}

	void Mesh::Unbind()
	{
		glDisableVertexAttribArray(0); VEGAM_CHECK_GL_ERROR;
		glBindVertexArray(0); VEGAM_CHECK_GL_ERROR;
	}
}