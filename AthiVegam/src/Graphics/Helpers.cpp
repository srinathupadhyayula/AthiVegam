#include "AthiVegam/Graphics/Helpers.h"
#include "AthiVegam/Log.h"
#include "glad/glad.h"
#include <string>

namespace AthiVegam::Graphics
{
	void CheckOpenGLError()
	{
		GLenum error = glGetError();
		auto shouldAssert = (error == GL_NO_ERROR)?true:false;

		while (error != GL_NO_ERROR)
		{
			std::string errorStr;
			switch (error)
			{
			case GL_INVALID_OPERATION:				errorStr = "INVALID_OPERATION";					break;
			case GL_INVALID_ENUM:					errorStr = "GL_INVALID_ENUM";					break;
			case GL_INVALID_VALUE:					errorStr = "GL_INVALID_VALUE";					break;
			case GL_OUT_OF_MEMORY:					errorStr = "GL_OUT_OF_MEMORY";					break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:	errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION";	break;
			default: break;
			}

			VEGAM_ERROR("OpenlGL Error: {}", errorStr.c_str());
			auto assertMsg = std::string("OpenGL Error! " + errorStr);
			VEGAM_ASSERT(shouldAssert, assertMsg);
			error = glGetError();
		}
	}
}