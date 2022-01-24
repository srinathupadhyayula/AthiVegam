#include "AthiVegam/Graphics/Shader.h"
#include "AthiVegam/Graphics/Helpers.h"

#include "AthiVegam/Log.h"

#include "glad/glad.h"

namespace AthiVegam::Graphics
{
	Shader::Shader(const std::string& vertex, const std::string& fragment)
	{
		auto status = GL_FALSE;;
		char errorLog[512];

		m_programId = glCreateProgram(); VEGAM_CHECK_GL_ERROR;

		auto vertexShaderId = glCreateShader(GL_VERTEX_SHADER); VEGAM_CHECK_GL_ERROR;
		auto fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER); VEGAM_CHECK_GL_ERROR;

		// Vertex Shader
		{
			auto glSource = vertex.c_str();
			glShaderSource(vertexShaderId, 1, &glSource, NULL); VEGAM_CHECK_GL_ERROR;
			glCompileShader(vertexShaderId); VEGAM_CHECK_GL_ERROR;
			glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &status); VEGAM_CHECK_GL_ERROR;
			if (status != GL_TRUE)
			{
				glGetShaderInfoLog(vertexShaderId, sizeof(errorLog), NULL, errorLog);
				VEGAM_ERROR("Vertex Shader compilation error: {}", errorLog);
			}
			else
			{
				glAttachShader(m_programId, vertexShaderId); VEGAM_CHECK_GL_ERROR;
			}
		}

		// Fragment Shader
		if(status == GL_TRUE)
		{
			auto glSource = fragment.c_str();
			glShaderSource(fragmentShaderId, 1, &glSource, NULL); VEGAM_CHECK_GL_ERROR;
			glCompileShader(fragmentShaderId); VEGAM_CHECK_GL_ERROR;
			glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &status); VEGAM_CHECK_GL_ERROR;
			if (status != GL_TRUE)
			{
				glGetShaderInfoLog(fragmentShaderId, sizeof(errorLog), NULL, errorLog); VEGAM_CHECK_GL_ERROR;
				VEGAM_ERROR("Fragment Shader compilation error: {}", errorLog);
			}
			else
			{
				glAttachShader(m_programId, fragmentShaderId); VEGAM_CHECK_GL_ERROR;
			}
		}

		VEGAM_ASSERT(status == GL_TRUE, "Error Compiling Shader");

		if (status == GL_TRUE)
		{
			glLinkProgram(m_programId); VEGAM_CHECK_GL_ERROR;
			glValidateProgram(m_programId); VEGAM_CHECK_GL_ERROR;
			glGetProgramiv(m_programId, GL_LINK_STATUS, &status); VEGAM_CHECK_GL_ERROR;
			if (status != GL_TRUE)
			{
				glGetProgramInfoLog(m_programId, sizeof(errorLog), NULL, errorLog); VEGAM_CHECK_GL_ERROR;
				VEGAM_ERROR("Shader link error: {}", errorLog);
				glDeleteProgram(m_programId); VEGAM_CHECK_GL_ERROR;
				m_programId = -1;
			}
		}

		glDeleteShader(vertexShaderId); VEGAM_CHECK_GL_ERROR;
		glDeleteShader(fragmentShaderId); VEGAM_CHECK_GL_ERROR;
	}

	Shader::~Shader()
	{
		glUseProgram(0); VEGAM_CHECK_GL_ERROR;
		glDeleteProgram(m_programId); VEGAM_CHECK_GL_ERROR;
	}

	void Shader::Bind()
	{
		glUseProgram(m_programId); VEGAM_CHECK_GL_ERROR;
	}

	void Shader::Unbind()
	{
		glUseProgram(0); VEGAM_CHECK_GL_ERROR;
	}

	void Shader::SetUniformInt(const std::string& name, int val)
	{
		Bind();
		glUniform1i(GetUniformLocation(name), val); VEGAM_CHECK_GL_ERROR;
	}

	void Shader::SetUniformInt2(const std::string& name, int val1, int val2)
	{
		Bind();
		glUniform2i(GetUniformLocation(name), val1, val2); VEGAM_CHECK_GL_ERROR;
	}

	void Shader::SetUniformInt3(const std::string& name, int val1, int val2, int val3)
	{
		Bind();
		glUniform3i(GetUniformLocation(name), val1, val2, val3); VEGAM_CHECK_GL_ERROR;
	}

	void Shader::SetUniformInt4(const std::string& name, int val1, int val2, int val3, int val4)
	{
		Bind();
		glUniform4i(GetUniformLocation(name), val1, val2, val3, val4); VEGAM_CHECK_GL_ERROR;
	}

	void Shader::SetUniformFloat(const std::string& name, float val)
	{
		Bind();
		glUniform1f(GetUniformLocation(name), val); VEGAM_CHECK_GL_ERROR;
	}

	void Shader::SetUniformFloat2(const std::string& name, float val1, float val2)
	{
		Bind();
		glUniform2f(GetUniformLocation(name), val1, val2); VEGAM_CHECK_GL_ERROR;
	}

	void Shader::SetUniformFloat3(const std::string& name, float val1, float val2, float val3)
	{
		Bind();
		glUniform3f(GetUniformLocation(name), val1, val2, val3); VEGAM_CHECK_GL_ERROR;
	}

	void Shader::SetUniformFloat4(const std::string& name, float val1, float val2, float val3, float val4)
	{
		Bind();
		glUniform4f(GetUniformLocation(name), val1, val2, val3, val4); VEGAM_CHECK_GL_ERROR;
	}

	int Shader::GetUniformLocation(const std::string& name)
	{
		auto it = m_uniformLocations.find(name);
		
		if (it == m_uniformLocations.end())
		{
			m_uniformLocations[name] = glGetUniformLocation(m_programId, name.c_str()); VEGAM_CHECK_GL_ERROR;
		}

		return m_uniformLocations[name];
	}
}