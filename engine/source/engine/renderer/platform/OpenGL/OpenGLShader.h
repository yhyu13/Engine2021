#pragma once

#include "../../Shader.h"
#include "engine/math/Geommath.h"
#include "engine/core/utility/TypeHelper.h"
#include "OpenGLUtil.h"

namespace AAAAgames {
	class OpenGLShader : public Shader
	{
	public:
		explicit OpenGLShader(const fs::path& computeShaderPath);
		explicit OpenGLShader(const fs::path& vertexShaderPath, const fs::path& fragmentShaderPath, const fs::path& geomtryShaderPath = "");
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetMat3(const std::string& name, const glm::mat3& matrix) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& matrix) override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
		virtual void SetIntV(const std::string& name, int count, const int* value) override;
		virtual void SetFloat3V(const std::string& name, int count, const glm::vec3* value) override;

		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& value);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& value);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& value);

		void UploadUniformIntV(const std::string& name, int count, const int* value);
		void UploadUniformFloat3V(const std::string& name, int count, const glm::vec3* value);

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

		uint32_t GetRendererID() { return m_RendererID; }

	private:
		GLint GetUniformLocationCached(const std::string& name);
		std::string GetShaderString(const fs::path& shaderSourcePath);
	private:
		A4GAMES_UnorderedMap<std::string, GLint> m_uniformLocationMap;
		uint32_t m_RendererID;
	};
}