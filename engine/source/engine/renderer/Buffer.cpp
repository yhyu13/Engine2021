#include "engine-precompiled-header.h"
#include "Buffer.h"
#include "Renderer2D.h"
#include "platform/OpenGL/OpenGLBuffer.h"

namespace longmarch {
	std::shared_ptr<IndexedIndirectCommandBuffer> IndexedIndirectCommandBuffer::Create(const void* data, size_t size)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return MemoryManager::Make_shared<OpenGLIndexedIndirectCommandBuffer>(data, size);
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::shared_ptr<VertexBuffer> VertexBuffer::Create(const void* data, size_t size)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return MemoryManager::Make_shared<OpenGLVertexBuffer>(data, size);
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::shared_ptr<IndexBuffer> IndexBuffer::Create(const void* data, size_t size, size_t elementSize)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return MemoryManager::Make_shared<OpenGLIndexBuffer>(data, size, elementSize);
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	void* UniformBuffer::GetUniformBufferMapping(size_t size, size_t offset)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return OpenGLUniformBuffer::GetUniformBufferMapping(size, offset);
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	void UniformBuffer::EndUniformBufferMapping()
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!");
		case RendererAPI::API::OpenGL: return OpenGLUniformBuffer::EndUniformBufferMapping();
		}

		ASSERT(false, "Unknown RendererAPI!");
	}

	std::shared_ptr<UniformBuffer> UniformBuffer::Create(const void* data, size_t size)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return MemoryManager::Make_shared<OpenGLUniformBuffer>(data, size);
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	void* ShaderStorageBuffer::GetShaderStorageBufferMapping(size_t size, size_t offset)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return OpenGLShaderStorageBuffer::GetShaderStorageBufferMapping(size, offset);
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	void ShaderStorageBuffer::EndShaderStorageBufferMapping()
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!");
		case RendererAPI::API::OpenGL: return OpenGLShaderStorageBuffer::EndShaderStorageBufferMapping();
		}

		ASSERT(false, "Unknown RendererAPI!");
	}

	std::shared_ptr<ShaderStorageBuffer> ShaderStorageBuffer::Create(const void* data, size_t size)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return MemoryManager::Make_shared<OpenGLShaderStorageBuffer>(data, size);
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::shared_ptr<FrameBuffer> FrameBuffer::Create(uint32_t width, uint32_t height, FrameBuffer::BUFFER_FORMAT format)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return MemoryManager::Make_shared<OpenGLFrameBuffer>(width, height, format);
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::shared_ptr<ShadowBuffer> ShadowBuffer::Create(uint32_t width, uint32_t height, ShadowBuffer::SHADOW_MAP_TYPE type)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:
		{
			switch (type)
			{
			case longmarch::ShadowBuffer::SHADOW_MAP_TYPE::BASIC:
				return MemoryManager::Make_shared<OpenGLShadowBuffer>(width, height);
			case longmarch::ShadowBuffer::SHADOW_MAP_TYPE::BASIC_COMPARE:
				return MemoryManager::Make_shared<OpenGLCompareShadowBuffer>(width, height);
			case longmarch::ShadowBuffer::SHADOW_MAP_TYPE::MOMENT4:
				return MemoryManager::Make_shared<OpenGLMSMShadowBuffer>(width, height);
			case longmarch::ShadowBuffer::SHADOW_MAP_TYPE::MOMENT4_CUBE:
				return MemoryManager::Make_shared<OpenGLMSMCubeShadowBuffer>(width, height);
			}
		}
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::shared_ptr<ShadowBuffer> ShadowBuffer::CreateArray(uint32_t width, uint32_t height, uint32_t depth, ShadowBuffer::SHADOW_MAP_TYPE type)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:
		{
			switch (type)
			{
			case longmarch::ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_BASIC:
				return MemoryManager::Make_shared<OpenGLShadowArrayBuffer>(width, height, depth);
			case longmarch::ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_COMPARE:
				return MemoryManager::Make_shared<OpenGLCompareShadowArrayBuffer>(width, height, depth);
			case longmarch::ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_MOMENT4:
				return MemoryManager::Make_shared<OpenGLMSMShadowArrayBuffer>(width, height, depth);
			}
		}
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::shared_ptr<SkyBoxBuffer> SkyBoxBuffer::Create(uint32_t width, uint32_t height, SkyBoxBuffer::BUFFER_FORMAT format)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return MemoryManager::Make_shared<OpenGLSkyBoxBuffer>(width, height, format);
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::shared_ptr<GBuffer> GBuffer::Create(uint32_t width, uint32_t height, GBUFFER_TYPE type)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:
			switch (type)
			{
			case longmarch::GBuffer::GBUFFER_TYPE::DEFAULT:
				return MemoryManager::Make_shared<OpenGLGBuffer>(width, height);
			case longmarch::GBuffer::GBUFFER_TYPE::THIN:
				return MemoryManager::Make_shared<OpenGLThinGBuffer>(width, height);
			}
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::shared_ptr<ComputeBuffer> ComputeBuffer::Create(uint32_t width, uint32_t height, ComputeBuffer::BUFFER_FORMAT format)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None: ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return MemoryManager::Make_shared<OpenGLComputeBuffer>(width, height, format);
		}

		ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}