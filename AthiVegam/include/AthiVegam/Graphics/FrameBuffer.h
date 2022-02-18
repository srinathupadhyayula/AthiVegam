#pragma once
#include <cstdint>

namespace AthiVegam::Graphics
{
	class Framebuffer
	{
	  public:
		Framebuffer(uint32_t width, uint32_t height);
		~Framebuffer();

		inline uint32_t GetFbo() const { return m_fbo; }
		inline uint32_t GetTextureId() const
		{
			return m_textureId;
		}
		inline uint32_t GetRenderBufferId() const
		{
			return m_renderBufferId;
		}
		inline void GetSize(uint32_t& width,
		                    uint32_t& height) const
		{
			width = m_width;
			height = m_height;
		}

		inline void SetClearColor(float r, float g, float b,
		                          float a)
		{
			m_clearColorR = r;
			m_clearColorG = g;
			m_clearColorB = b;
			m_clearColorA = a;
		}
		inline void GetClearColor(float& r, float& g,
		                          float& b, float& a)
		{
			r = m_clearColorR;
			g = m_clearColorG;
			b = m_clearColorB;
			a = m_clearColorA;
		}

	  private:
		uint32_t m_fbo;
		uint32_t m_textureId;
		uint32_t m_renderBufferId;
		uint32_t m_width, m_height;
		float m_clearColorR, m_clearColorG, m_clearColorB,
		    m_clearColorA; /* Replace with glm vectors */
	};
} // namespace AthiVegam::Graphics
