#pragma once

#include "../../Buffer.h"
#include "OpenGLUtil.h"

namespace longmarch {
	class OpenGLIndexedIndirectCommandBuffer :public IndexedIndirectCommandBuffer
	{
	public:
		OpenGLIndexedIndirectCommandBuffer(const void* data, size_t size);

		virtual ~OpenGLIndexedIndirectCommandBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void UpdateBufferData(const void* data, size_t size) override;

		inline virtual size_t GetCapacity() const override { return m_Capcity; };
		inline virtual size_t GetCount() const override { return m_Count; }
		inline virtual size_t GetElementSize() const override { return sizeof(DrawIndexedIndirectCommand); }
		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }

	private:
		uint32_t m_RendererID;
		size_t m_Count;
		size_t m_Capcity;
	};

	class OpenGLVertexBuffer :public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(const void* data, size_t size);

		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AppendBufferData(const std::shared_ptr<VertexBuffer>& read_buffer) override;
		virtual void AppendBufferData(const void* data, size_t size) override;

		virtual void CopyBufferData(size_t write_offset, const std::shared_ptr<VertexBuffer>& read_buffer, size_t read_offset) override;
		virtual void UpdateBufferData(const void* data, size_t size) override;
		virtual void UpdateBufferSubData(const void* data, size_t size, size_t write_offset) override;

		inline virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		inline virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

		inline virtual size_t GetCapacity() const override { return m_Capcity; };
		inline virtual size_t GetCount() const override { return m_Count; }
		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }

	private:
		BufferLayout m_Layout;
		uint32_t m_RendererID;
		size_t m_Count;
		size_t m_Capcity;
	};

	class OpenGLIndexBuffer :public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(const void* indices, size_t size, size_t elementSize);

		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AppendBufferData(const std::shared_ptr<IndexBuffer>& read_buffer) override;
		virtual void AppendBufferData(const void* data, size_t size) override;

		virtual void CopyBufferData(size_t write_offset, const std::shared_ptr<IndexBuffer>& read_buffer, size_t read_offset) override;
		virtual void UpdateBufferData(const void* data, size_t size) override;
		virtual void UpdateBufferSubData(const void* data, size_t size, size_t write_offset) override;

		inline virtual size_t GetCapacity() const override { return m_Capcity; };
		inline virtual size_t GetCount() const override { return m_Count; }
		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }
	private:
		uint32_t m_RendererID;
		size_t m_Count;
		size_t m_Capcity;
	};

	class OpenGLUniformBuffer :public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(const void* data, size_t size);

		virtual ~OpenGLUniformBuffer();

		virtual void Bind(uint32_t slot) const override;
		virtual void Unbind() const override;
		virtual void UpdateBufferData(const void* data, size_t size) override;

		inline virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		inline virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

		static void* GL_GetUniformBufferMapping(size_t size, size_t offset);
		static void GL_EndUniformBufferMapping();

	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};

	class OpenGLShaderStorageBuffer :public ShaderStorageBuffer
	{
	public:
		OpenGLShaderStorageBuffer(const void* data, size_t size);

		virtual ~OpenGLShaderStorageBuffer();

		virtual void Bind(uint32_t slot) const override;
		virtual void Unbind() const override;
		virtual void UpdateBufferData(const void* data, size_t size) override;

		inline virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		inline virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

		static void* GL_GetShaderStorageBufferMapping(size_t size, size_t offset);
		static void GL_EndShaderStorageBufferMapping();
	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};

	class OpenGLFrameBuffer :public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(uint32_t width, uint32_t height, FrameBuffer::BUFFER_FORMAT format);

		virtual ~OpenGLFrameBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void BindTexture(uint32_t slot) const override;
		virtual void GenerateMipmaps() const override;

		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }
		inline virtual uint32_t GetRenderTargetID() const override { return m_RenderTargetID; }

	private:
		uint32_t m_RendererID;
		uint32_t m_RenderTargetID;
		uint32_t m_DepthID;
	};

	class OpenGLShadowBuffer :public ShadowBuffer
	{
	public:
		OpenGLShadowBuffer(uint32_t width, uint32_t height);

		virtual ~OpenGLShadowBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void BindTexture(uint32_t slot) const override;

		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }
		inline virtual uint32_t GetRenderTargetID() const override { return m_RenderTargetID; }

	private:
		uint32_t m_RendererID;
		uint32_t m_RenderTargetID;
	};

	class OpenGLMSMShadowBuffer :public ShadowBuffer
	{
	public:
		OpenGLMSMShadowBuffer(uint32_t width, uint32_t height);

		virtual ~OpenGLMSMShadowBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void BindTexture(uint32_t slot) const override;

		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }
		inline virtual uint32_t GetRenderTargetID() const override { return m_RenderTargetID; }

	private:
		uint32_t m_RendererID;
		uint32_t m_DepthID;
		uint32_t m_RenderTargetID;
	};

	class OpenGLMSMCubeShadowBuffer :public ShadowBuffer
	{
	public:
		OpenGLMSMCubeShadowBuffer(uint32_t width, uint32_t height);

		virtual ~OpenGLMSMCubeShadowBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void BindTexture(uint32_t slot) const override;

		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }
		inline virtual uint32_t GetRenderTargetID() const override { return m_RenderTargetID; }

	private:
		uint32_t m_RendererID;
		uint32_t m_DepthID;
		uint32_t m_RenderTargetID;
	};

	class OpenGLShadowArrayBuffer :public ShadowBuffer
	{
	public:
		OpenGLShadowArrayBuffer(uint32_t width, uint32_t height, uint32_t depth);

		virtual ~OpenGLShadowArrayBuffer();

		virtual void Bind() const override;
		virtual void BindLayer(uint32_t slot) const override;
		virtual void Unbind() const override;
		virtual void BindTexture(uint32_t slot) const override;

		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }
		inline virtual uint32_t GetRenderTargetID() const override { return m_RenderTargetID; }

	private:
		uint32_t m_RendererID;
		uint32_t m_DepthID;
		uint32_t m_RenderTargetID;
	};

	class OpenGLMSMShadowArrayBuffer :public ShadowBuffer
	{
	public:
		OpenGLMSMShadowArrayBuffer(uint32_t width, uint32_t height, uint32_t depth);

		virtual ~OpenGLMSMShadowArrayBuffer();

		virtual void Bind() const override;
		virtual void BindLayer(uint32_t slot) const override;
		virtual void Unbind() const override;
		virtual void BindTexture(uint32_t slot) const override;

		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }
		inline virtual uint32_t GetRenderTargetID() const override { return m_RenderTargetID; }

	private:
		uint32_t m_RendererID;
		uint32_t m_DepthID;
		uint32_t m_RenderTargetID;
	};

	class OpenGLSkyBoxBuffer :public SkyBoxBuffer
	{
	public:
		OpenGLSkyBoxBuffer(uint32_t width, uint32_t height, SkyBoxBuffer::BUFFER_FORMAT type);

		virtual ~OpenGLSkyBoxBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void BindTexture(uint32_t slot) const override;
		virtual void BindMipMap(uint32_t level) const override;
		virtual void GenerateMipmaps() const override;

		inline virtual uint32_t GetMaxMipMapLevel() const { return m_max_level; };
		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }
		inline virtual uint32_t GetRenderTargetID() const override { return m_RenderTargetID; }

	private:
		uint32_t m_max_level;
		uint32_t m_DepthID;
		uint32_t m_RendererID;
		uint32_t m_RenderTargetID;
	};

	class OpenGLGBuffer : public GBuffer
	{
	public:
		OpenGLGBuffer(uint32_t width, uint32_t height);

		virtual ~OpenGLGBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		inline uint32_t GetRendererID() const { return m_RendererID; }
		virtual uint32_t GetTexutureID(GBUFFER_TEXTURE_TYPE tex) const override;

		virtual void BindTextures(const std::vector<GBUFFER_TEXTURE_TYPE>& texToBind, uint32_t offset) const override;

	private:
		uint32_t m_RendererID;
		uint32_t m_DepthID;
		uint32_t m_RenderNormal_VelocityID;
		uint32_t m_RenderAlbedo_EmssiveID;
		uint32_t m_RenderAO_Metallic_RoughnessID;
	};

	class OpenGLThinGBuffer : public GBuffer
	{
	public:
		OpenGLThinGBuffer(uint32_t width, uint32_t height);

		virtual ~OpenGLThinGBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		inline uint32_t GetRendererID() const { return m_RendererID; }
		virtual uint32_t GetTexutureID(GBUFFER_TEXTURE_TYPE tex) const override;

		virtual void BindTextures(const std::vector<GBUFFER_TEXTURE_TYPE>& texToBind, uint32_t offset) const override;

	private:
		uint32_t m_RendererID;
		uint32_t m_DepthID;
		uint32_t m_RenderNormal_VelocityID;
	};

	class OpenGLComputeBuffer :public ComputeBuffer
	{
	public:
		OpenGLComputeBuffer(uint32_t width, uint32_t height, ComputeBuffer::BUFFER_FORMAT type);

		virtual ~OpenGLComputeBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void BindTexture(uint32_t slot, ComputeBuffer::TEXTURE_BIND_MODE mode) const override;

		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }
		inline virtual uint32_t GetRenderTargetID() const override { return m_RenderTargetID; }

	private:
		uint32_t m_RendererID;
		uint32_t m_RenderTargetID;
	};
}