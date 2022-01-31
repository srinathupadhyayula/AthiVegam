#pragma once

namespace AthiVegam::Graphics
{
	void CheckOpenGLError();
	//{
	//	GLenum error = glGetError();
	//	bool shouldAssert = error != GL_NO_ERROR;
	//
	//	while (error != GL_NO_ERROR)
	//	{
	//		std::string errorStr;
	//		switch (error)
	//		{
	//		case GL_INVALID_OPERATION:				errorStr
	//= "INVALID_OPERATION";					break;
	// case GL_INVALID_ENUM:					errorStr =
	// "GL_INVALID_ENUM";
	// break; 		case GL_INVALID_VALUE:
	// errorStr = "GL_INVALID_VALUE"; break; case
	// GL_OUT_OF_MEMORY:					errorStr =
	// "GL_OUT_OF_MEMORY";
	// break; 		case GL_INVALID_FRAMEBUFFER_OPERATION:
	// errorStr =
	//"GL_INVALID_FRAMEBUFFER_OPERATION";	break;
	// default: break;
	//		}
	//
	//		VEGAM_ERROR("OpenlGL Error: {}",
	// errorStr.c_str()); 		error = glGetError();
	//	}
	//
	//	VEGAM_ASSERT(shouldAssert, "OpenGL Error!");
	//}
} // namespace AthiVegam::Graphics

#ifndef VEGAM_CHECK_GL_ERROR
#ifndef AV_CONFIG_RELEASE
#define VEGAM_CHECK_GL_ERROR                               \
	AthiVegam::Graphics::CheckOpenGLError();
#else
#define VEGAM_CHECK_GL_ERROR (void)0
#endif // AV_CONFIG_RELEASE
#endif
