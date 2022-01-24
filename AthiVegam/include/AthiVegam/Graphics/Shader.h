#pragma once

#include <string>
#include <unordered_map>

namespace AthiVegam::Graphics
{
	class Shader
	{
	public:
		Shader(const std::string& vertex, const std::string& fragment);
		~Shader();

		void Bind();
		void Unbind();

		void SetUniformInt(const std::string& name, int val);
		void SetUniformInt2(const std::string& name, int val1, int val2);
		void SetUniformInt3(const std::string& name, int val1, int val2, int val3);
		void SetUniformInt4(const std::string& name, int val1, int val2, int val3, int val4);
		void SetUniformFloat(const std::string& name, float val);
		void SetUniformFloat2(const std::string& name, float val1, float val2);
		void SetUniformFloat3(const std::string& name, float val1, float val2, float val3);
		void SetUniformFloat4(const std::string& name, float val1, float val2, float val3, float val4);

	private:
		int GetUniformLocation(const std::string& name);

	private:
		uint32_t m_programId;
		std::unordered_map<std::string, int> m_uniformLocations;
	};
}