#include "engine-precompiled-header.h"
#include "OpenGLShader.h"
#include "engine/core/utility/ShaderInclude.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace longmarch 
{
	OpenGLShader::OpenGLShader(const fs::path& computeShaderPath)
	{
		GLint isCompiled = 0;
		GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);

		const std::string computeShaderString = GetShaderString(computeShaderPath);
		const GLchar* source2 = computeShaderString.c_str();
		glShaderSource(computeShader, 1, &source2, 0);
		glCompileShader(computeShader);
		glGetShaderiv(computeShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(computeShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(computeShader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(computeShader);

			ENGINE_ERROR("{0}", infoLog.data());
			ASSERT(false, "Compoute shader compilation failure: " + computeShaderPath.string());

			return;
		}

		m_RendererID = glCreateProgram();
		GLuint program = m_RendererID;

		glAttachShader(program, computeShader);
		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(program);
			glDeleteShader(computeShader);

			ENGINE_ERROR("{0}", infoLog.data());
			ASSERT(false, "Shader link failure");

			return;
		}
		glDetachShader(program, computeShader);

		ENGINE_INFO("Compoute shader compilation success: " + computeShaderPath.string());
	}

	OpenGLShader::OpenGLShader(const fs::path& vertexShaderPath, const fs::path& fragmentShaderPath, const fs::path& geomtryShaderPath)
	{
		// Compile vertex shader
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		const std::string vertexShaderString = GetShaderString(vertexShaderPath);
		const GLchar* source = vertexShaderString.c_str();
		glShaderSource(vertexShader, 1, &source, 0);
		glCompileShader(vertexShader);
		GLint isCompiled = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(vertexShader);

			ENGINE_ERROR("{0}", infoLog.data());
			ASSERT(false, "Vertex shader compilation failure: " + vertexShaderPath.string());

			return;
		}

		// Try to compile geomtry shader
		GLuint geomtryShader = 0;
		bool hasGeomtryShader = (geomtryShaderPath != "");
		if (hasGeomtryShader)
		{
			geomtryShader = glCreateShader(GL_GEOMETRY_SHADER);
			const std::string geomtryShaderString = GetShaderString(geomtryShaderPath);
			const GLchar* source2 = geomtryShaderString.c_str();
			glShaderSource(geomtryShader, 1, &source2, 0);
			glCompileShader(geomtryShader);
			glGetShaderiv(geomtryShader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(geomtryShader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(geomtryShader, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(geomtryShader);

				ENGINE_ERROR("{0}", infoLog.data());
				ASSERT(false, "Geomtry shader compilation failure: " + geomtryShaderPath.string());

				return;
			}
		}

		// Compile fragment shader
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		const std::string fragmentShaderString = GetShaderString(fragmentShaderPath);
		const GLchar* source2 = fragmentShaderString.c_str();
		glShaderSource(fragmentShader, 1, &source2, 0);
		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
			
			glDeleteShader(fragmentShader);
			
			ENGINE_ERROR("{0}", infoLog.data());
			ASSERT(false, "Fragment shader compilation failure: " + fragmentShaderPath.string());
			
			return;
		}

		// Create shader program and link
		m_RendererID = glCreateProgram();
		GLuint program = m_RendererID;
		
		glAttachShader(program, vertexShader);
		if (hasGeomtryShader)
		{
			glAttachShader(program, geomtryShader);
		}
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(program);
			glDeleteShader(vertexShader);
			if (hasGeomtryShader)
			{
				glDetachShader(program, geomtryShader);
			}
			glDeleteShader(fragmentShader);

			ENGINE_ERROR("{0}", infoLog.data());
			ASSERT(false, "Shader link failure: " + fragmentShaderPath.string());

			return;
		}

		glDetachShader(program, vertexShader);
		if (hasGeomtryShader)
		{
			glDetachShader(program, geomtryShader);
		}
		glDetachShader(program, fragmentShader);

		ENGINE_DEBUG("Shader compilation success: " + fragmentShaderPath.string());
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
		GLCHECKERROR;
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& matrix)
	{
		UploadUniformMat3(name, matrix);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& matrix)
	{
		UploadUniformMat4(name, matrix);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		UploadUniformFloat2(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		UploadUniformFloat4(name, value);
	}

	void OpenGLShader::SetIntV(const std::string& name, int count, const int* value)
	{
		UploadUniformIntV(name, count, value);
	}

	void OpenGLShader::SetFloat3V(const std::string& name, int count, const glm::vec3* value)
	{
		UploadUniformFloat3V(name, count, value);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = GetUniformLocationCached(name);
		if (location != -1)
		{
			glUniform1i(location, value);
		}
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = GetUniformLocationCached(name);
		if (location != -1)
		{
			glUniform1f(location, value);
		}
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = GetUniformLocationCached(name);
		if (location != -1)
		{
			glUniform2f(location, value.x, value.y);
		}
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = GetUniformLocationCached(name);
		if (location != -1)
		{
			glUniform3f(location, value.x, value.y, value.z);
		}
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = GetUniformLocationCached(name);
		if (location != -1)
		{
			glUniform4f(location, value.x, value.y, value.z, value.w);
		}
	}

	void OpenGLShader::UploadUniformIntV(const std::string& name, int count, const int* value)
	{
		GLint location = GetUniformLocationCached(name);
		if (location != -1)
		{
			glUniform1iv(location, count, value);
		}
	}

	void OpenGLShader::UploadUniformFloat3V(const std::string& name, int count, const glm::vec3* value)
	{
		GLint location = GetUniformLocationCached(name);
		if (location != -1)
		{
			glUniform3fv(location, count, glm::value_ptr(*value));
		}
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = GetUniformLocationCached(name);
		if (location != -1)
		{
			glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		}
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = GetUniformLocationCached(name);
		if (location != -1)
		{
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		}
	}

	GLint OpenGLShader::GetUniformLocationCached(const std::string& name)
	{
		GLint location;
		if (const auto& it = m_uniformLocationMap.find(name); it != m_uniformLocationMap.end()) [[likely]]
		{
			location = it->second;
		}
		else [[unlikely]]
		{
			location = glGetUniformLocation(m_RendererID, name.c_str());
			m_uniformLocationMap[name] = location;
		}
		return location;
	}

	std::string OpenGLShader::GetShaderString(const fs::path& shaderSourcePath)
	{
		std::string shaderCode;
		try
		{
			shaderCode = LongMarch_ShaderInclude::load(shaderSourcePath, 0);
		}
		catch (std::ifstream::failure e)
		{
			ENGINE_ERROR("{0}", "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ : " + shaderSourcePath.string());
		}

		return shaderCode;
	}
}