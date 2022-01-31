#pragma once

#include <cstdint>

namespace AthiVegam::Graphics
{
	class Mesh
	{
	  public:
		Mesh(float* vertexArray, uint32_t vertexCount,
		     uint32_t dimensions);
		Mesh(float* vertexArray, uint32_t vertexCount,
		     uint32_t dimensions, uint32_t* elementArray,
		     uint32_t elementCount);
		~Mesh();

		void Bind();
		void Unbind();

		inline auto GetVertexCount() const
		{
			return m_vertexCount;
		}
		inline auto GetElementCount() const
		{
			return m_elementCount;
		}

	  private:
		uint32_t m_vertexCount;
		uint32_t m_elementCount;
		uint32_t m_vao;
		uint32_t m_ebo;
		uint32_t m_positionsVbo;
	};
} // namespace AthiVegam::Graphics
