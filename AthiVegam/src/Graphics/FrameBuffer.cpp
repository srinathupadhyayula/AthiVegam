#include "AthiVegam/Graphics/FrameBuffer.h"

#include "AthiVegam/Graphics/Helpers.h"
#include "AthiVegam/Log.h"

#include <glad/glad.h>

namespace AthiVegam::Graphics
{
	Framebuffer::Framebuffer(uint32_t width,
	                         uint32_t height)
	    : m_fbo(0)
	    , m_textureId(0)
	    , m_renderBufferId(0)
	    , m_width(width)
	    , m_height(height)
	    , m_clearColorR(1)
	    , m_clearColorG(1)
	    , m_clearColorB(1)
	    , m_clearColorA(1)
	{
		glGenFramebuffers(1, &m_fbo);
		VEGAM_CHECK_GL_ERROR;
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		VEGAM_CHECK_GL_ERROR;

		/* Create Color Texture */
		glGenTextures(1, &m_textureId);
		VEGAM_CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		VEGAM_CHECK_GL_ERROR;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width,
		             m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		             nullptr);
		VEGAM_CHECK_GL_ERROR;
		glTexParameteri(GL_TEXTURE_2D,
		                GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		VEGAM_CHECK_GL_ERROR;
		glTexParameteri(GL_TEXTURE_2D,
		                GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		VEGAM_CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, 0);
		VEGAM_CHECK_GL_ERROR;
		glFramebufferTexture2D(
		    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		    GL_TEXTURE_2D, m_textureId, 0);
		VEGAM_CHECK_GL_ERROR;

		/* Create depth/stencil renderbuffer */
		glGenRenderbuffers(1, &m_renderBufferId);
		VEGAM_CHECK_GL_ERROR;
		glBindRenderbuffer(GL_RENDERBUFFER,
		                   m_renderBufferId);
		VEGAM_CHECK_GL_ERROR;
		glRenderbufferStorage(GL_RENDERBUFFER,
		                      GL_DEPTH24_STENCIL8, m_width,
		                      m_height);
		VEGAM_CHECK_GL_ERROR;
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(
		    GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
		    GL_RENDERBUFFER, m_renderBufferId);
		VEGAM_CHECK_GL_ERROR;

		/* Check for completeness */

		int32_t completeStatus =
		    glCheckFramebufferStatus(GL_FRAMEBUFFER);
		VEGAM_CHECK_GL_ERROR;
		if (completeStatus != GL_FRAMEBUFFER_COMPLETE)
		{
			VEGAM_ERROR("Failed to create framebuffer. "
			            "Complete status: {}",
			            completeStatus);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		VEGAM_CHECK_GL_ERROR;
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers(1, &m_fbo);
		m_fbo = 0;
		m_textureId = 0;
		m_renderBufferId = 0;
	}
} // namespace AthiVegam::Graphics
