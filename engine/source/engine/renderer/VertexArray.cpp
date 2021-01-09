#include "engine-precompiled-header.h"
#include "VertexArray.h"
#include "Renderer2D.h"
#include "platform/OpenGL/OpenGLVertexArray.h"

namespace longmarch {

	std::shared_ptr<VertexArray> VertexArray::Create()
	{
		switch (Renderer2D::GetAPI())
		{
			case RendererAPI::API::None: ASSERT(false, "RendererAPI::API::None is currently not supported!");
			case RendererAPI::API::OpenGL: return MemoryManager::Make_shared<OpenGLVertexArray>();
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}