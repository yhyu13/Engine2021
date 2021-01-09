#include "engine-precompiled-header.h"
#include "Texture.h"
#include "Renderer2D.h"
#include "platform/OpenGL/OpenGLTexture.h"

namespace longmarch {
	std::shared_ptr<Texture2D> Texture2D::Create(Texture::Setting data)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None:   ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return MemoryManager::Make_shared<OpenGLTexture2D>(data);
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::shared_ptr<Texture2D> Texture2D::LoadFromFile(const fs::path& path)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None:   ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return MemoryManager::Make_shared<OpenGLTexture2D>(path);
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}