#pragma once

#include "engine/math/Geommath.h"
#include "engine/core/EngineCore.h"

namespace longmarch {
	class ENGINE_API Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetMat3(const std::string& name, const glm::mat3& matrix) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& matrix) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetIntV(const std::string& name, int count, const int* value) = 0;
		virtual void SetFloat3V(const std::string& name, int count, const glm::vec3* value) = 0;

		virtual uint32_t GetShaderID() = 0;

		static std::shared_ptr<Shader> Create(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const std::string& geomtryShaderPath = "");
		static std::shared_ptr<Shader> Create(const std::string& computeShaderPath);
	};
}
