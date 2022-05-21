#include "engine-precompiled-header.h"
#include "Shader.h"
#include "Renderer2D.h"
#include "platform/OpenGL/OpenGLShader.h"

namespace longmarch {
	std::shared_ptr<Shader> Shader::Create(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const std::string& geomtryShaderPath)
	{
		auto vert = FileSystem::ResolveProtocol(vertexShaderPath);
		auto frag = FileSystem::ResolveProtocol(fragmentShaderPath);
		auto geom = FileSystem::ResolveProtocol(geomtryShaderPath);
		switch (RendererAPI::WhichAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return MemoryManager::Make_shared<OpenGLShader>(vert, frag, geom);
		}

		ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
	std::shared_ptr<Shader> Shader::Create(const std::string& computeShaderPath)
	{
		auto comp = FileSystem::ResolveProtocol(computeShaderPath);
		switch (RendererAPI::WhichAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return MemoryManager::Make_shared<OpenGLShader>(comp);
		}

		ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}
