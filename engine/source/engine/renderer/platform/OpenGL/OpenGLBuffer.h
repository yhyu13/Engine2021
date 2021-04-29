#pragma once

#include "../../Buffer.h"
#include "OpenGLUtil.h"

namespace longmarch 
{
	class OpenGLVertexBuffer final : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(const void* data, size_t size);
		~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AppendBufferData(const std::shared_ptr<VertexBuffer>& read_buffer) override;
		virtual void AppendBufferData(const void* data, size_t size) override;

		virtual void CopyBufferData(size_t write_offset, const std::shared_ptr<VertexBuffer>& read_buffer, size_t read_offset) override;
		virtual void UpdateBufferData(const void* data, size_t size) override;
		virtual void UpdateBufferSubData(const void* data, size_t size, size_t write_offset) override;

		inline virtual const VertexBufferLayout& GetLayout() const override { return m_Layout; }
		inline virtual void SetLayout(const VertexBufferLayout& layout) override { m_Layout = layout; }

		inline virtual size_t GetCapacity() const override { return m_Capcity; };
		inline virtual size_t GetCount() const override { return m_Count; }

	private:
		VertexBufferLayout m_Layout;
		size_t m_Count;
		size_t m_Capcity;
	};

	class OpenGLIndexBuffer final : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(const void* indices, size_t size, size_t elementSize);
		~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AppendBufferData(const std::shared_ptr<IndexBuffer>& read_buffer) override;
		virtual void AppendBufferData(const void* data, size_t size) override;

		virtual void CopyBufferData(size_t write_offset, const std::shared_ptr<IndexBuffer>& read_buffer, size_t read_offset) override;
		virtual void UpdateBufferData(const void* data, size_t size) override;
		virtual void UpdateBufferSubData(const void* data, size_t size, size_t write_offset) override;

		inline virtual size_t GetCapacity() const override { return m_Capcity; };
		inline virtual size_t GetCount() const override { return m_Count; }

	private:
		size_t m_Count;
		size_t m_Capcity;
	};

	class OpenGLUniformBuffer final : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(const void* data, size_t size);
		~OpenGLUniformBuffer();

		virtual void Bind(uint32_t slot) const override;
		virtual void Unbind() const override;
		virtual void UpdateBufferData(const void* data, size_t size) override;

		inline virtual const VertexBufferLayout& GetLayout() const override { return m_Layout; }
		inline virtual void SetLayout(const VertexBufferLayout& layout) override { m_Layout = layout; }

		static void* GetUniformBufferMapping(size_t size, size_t offset);
		static void EndUniformBufferMapping();

	private:
		VertexBufferLayout m_Layout;
	};

	class OpenGLShaderStorageBuffer final : public ShaderStorageBuffer
	{
	public:
		OpenGLShaderStorageBuffer(const void* data, size_t size);
		~OpenGLShaderStorageBuffer();

		virtual void Bind(uint32_t slot) const override;
		virtual void Unbind() const override;
		virtual void UpdateBufferData(const void* data, size_t size) override;

		inline virtual const VertexBufferLayout& GetLayout() const override { return m_Layout; }
		inline virtual void SetLayout(const VertexBufferLayout& layout) override { m_Layout = layout; }

		static void* GetShaderStorageBufferMapping(size_t size, size_t offset);
		static void EndShaderStorageBufferMapping();

	private:
		VertexBufferLayout m_Layout;
	};

	class OpenGLIndexedIndirectCommandBuffer final : public IndexedIndirectCommandBuffer
	{
	public:
		OpenGLIndexedIndirectCommandBuffer(const void* data, size_t size);
		~OpenGLIndexedIndirectCommandBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void UpdateBufferData(const void* data, size_t size) override;

		inline virtual size_t GetCapacity() const override { return m_Capcity; };
		inline virtual size_t GetCount() const override { return m_Count; }
		inline virtual size_t GetElementSize() const override { return sizeof(DrawIndexedIndirectCommand); }

	private:
		size_t m_Count;
		size_t m_Capcity;
	};

	class OpenGLBaseTextureBuffer : public virtual BaseTextureBuffer
	{
	public:
		virtual void Bind() const;
		virtual void Unbind() const;
		virtual void BindTexture(uint32_t slot);
	};

	class OpenGLFrameBuffer final : public FrameBuffer, public OpenGLBaseTextureBuffer
	{
	public:
		OpenGLFrameBuffer(uint32_t width, uint32_t height, FrameBuffer::BUFFER_FORMAT format);
		~OpenGLFrameBuffer();

		virtual void GenerateMipMapLevel() const override;
	
	private:
		uint32_t m_DepthID;
	};

	class OpenGLShadowBuffer final : public ShadowBuffer, public OpenGLBaseTextureBuffer
	{
	public:
		OpenGLShadowBuffer(uint32_t width, uint32_t height);
		~OpenGLShadowBuffer();

	private:
		uint32_t m_DepthID;
	};

	class OpenGLCompareShadowBuffer final : public ShadowBuffer, public OpenGLBaseTextureBuffer
	{
	public:
		OpenGLCompareShadowBuffer(uint32_t width, uint32_t height);
		~OpenGLCompareShadowBuffer();

	private:
		uint32_t m_DepthID;
	};

	class OpenGLMSMShadowBuffer final : public ShadowBuffer, public OpenGLBaseTextureBuffer
	{
	public:
		OpenGLMSMShadowBuffer(uint32_t width, uint32_t height);
		~OpenGLMSMShadowBuffer();

	private:
		uint32_t m_DepthID;
	};

	class OpenGLMSMCubeShadowBuffer final : public ShadowBuffer, public OpenGLBaseTextureBuffer
	{
	public:
		OpenGLMSMCubeShadowBuffer(uint32_t width, uint32_t height);
		~OpenGLMSMCubeShadowBuffer();

	private:
		uint32_t m_DepthID;
	};

	class OpenGLShadowArrayBuffer final : public ShadowBuffer, public OpenGLBaseTextureBuffer
	{
	public:
		OpenGLShadowArrayBuffer(uint32_t width, uint32_t height, uint32_t depth);
		~OpenGLShadowArrayBuffer();
		
		virtual void BindLayer(uint32_t slot) const override;

	private:
		uint32_t m_DepthID;
	};

	class OpenGLCompareShadowArrayBuffer final : public ShadowBuffer, public OpenGLBaseTextureBuffer
	{
	public:
		OpenGLCompareShadowArrayBuffer(uint32_t width, uint32_t height, uint32_t depth);
		~OpenGLCompareShadowArrayBuffer();

		virtual void BindLayer(uint32_t slot) const override;

	private:
		uint32_t m_DepthID;
	};

	class OpenGLMSMShadowArrayBuffer final : public ShadowBuffer, public OpenGLBaseTextureBuffer
	{
	public:
		OpenGLMSMShadowArrayBuffer(uint32_t width, uint32_t height, uint32_t depth);
		~OpenGLMSMShadowArrayBuffer();

		virtual void BindLayer(uint32_t slot) const override;

	private:
		uint32_t m_DepthID;
	};

	class OpenGLSkyBoxBuffer final : public SkyBoxBuffer, public OpenGLBaseTextureBuffer
	{
	public:
		OpenGLSkyBoxBuffer(uint32_t width, uint32_t height, SkyBoxBuffer::BUFFER_FORMAT type);
		~OpenGLSkyBoxBuffer();

		virtual void BindMipMapLevel(uint32_t level) const override;
		virtual void GenerateMipMapLevel() const override;
	};

	class OpenGLGBuffer final : public GBuffer, public OpenGLBaseTextureBuffer
	{
	public:
		OpenGLGBuffer(uint32_t width, uint32_t height);
		~OpenGLGBuffer();

		virtual uint32_t GetGBufferTexutureID(GBUFFER_TEXTURE_TYPE tex) const override;
		virtual void BindTextures(const std::vector<GBUFFER_TEXTURE_TYPE>& texToBind, uint32_t offset) const override;

	private:
		uint32_t m_DepthID;
		uint32_t m_RenderNormalID;
		uint32_t m_RenderVelocityID;
		uint32_t m_Render_Albedo_Emssive_ID;
		uint32_t m_Render_AO_Metallic_Roughness_ID;
	};

	class OpenGLThinGBuffer final : public GBuffer, public OpenGLBaseTextureBuffer
	{
	public:
		OpenGLThinGBuffer(uint32_t width, uint32_t height);
		~OpenGLThinGBuffer();

		virtual uint32_t GetGBufferTexutureID(GBUFFER_TEXTURE_TYPE tex) const override;
		virtual void BindTextures(const std::vector<GBUFFER_TEXTURE_TYPE>& texToBind, uint32_t offset) const override;

	private:
		uint32_t m_DepthID;
		uint32_t m_RenderNormalID;
		uint32_t m_RenderVelocityID;
	};

	class OpenGLComputeBuffer final : public ComputeBuffer, public OpenGLBaseTextureBuffer
	{
	public:
		OpenGLComputeBuffer(uint32_t width, uint32_t height, ComputeBuffer::BUFFER_FORMAT type);
		~OpenGLComputeBuffer();

		virtual void BindImage(uint32_t slot, BaseTextureBuffer::IMAGE_BIND_MODE mode) const override;
	};

	class OpenGLVoxelBuffer final : public VoxelBuffer, public OpenGLBaseTextureBuffer
	{
	public:
		OpenGLVoxelBuffer(uint32_t width, uint32_t height, uint32_t depth, VoxelBuffer::BUFFER_TYPE type);
		~OpenGLVoxelBuffer();

		virtual void BindImage(uint32_t slot, BaseTextureBuffer::IMAGE_BIND_MODE mode) const override;
		virtual void BindImage(uint32_t slot, BaseTextureBuffer::IMAGE_BIND_MODE mode, VoxelBuffer::BUFFER_FORMAT format) const override;
		virtual void GenerateMipMapLevel() const override;
	};
}