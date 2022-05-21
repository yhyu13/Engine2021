#pragma once
#include "../../Texture.h"
#include "VulkanUtil.h"

#include <vulkan/vulkan.h>

namespace longmarch 
{
	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(Texture::Setting data);
		VulkanTexture2D(const fs::path& path);
		virtual ~VulkanTexture2D();

		inline virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		inline virtual uint32_t GetRenderTargetID() const override { return m_RenderTargetID; }
		inline virtual bool IsFloatType() const override { return m_float_type; }
		virtual uint32_t GetMaxMipMapLevel() const { return m_max_level; }
		inline virtual uint32_t GetTextureRowCount() const override { return m_rows; };

		virtual void BindTexture(uint32_t slot = 0) const override;

		//! Methods below are useful to attach a texture to a framebuffer, then read the rendered result back
		//! Always Bind FBO
		virtual void AttachToFrameBuffer() const override;
		//! Always Bind FBO
		virtual void WriteToPNG(const fs::path& path) const override;
		//! Always Bind FBO
		virtual void WriteToHDR(const fs::path& path) const override;
		//! Always Bind FBO
		virtual void ReadTexture(void* tex) const override;

	private:
		bool m_float_type{ false };
		uint32_t m_max_level;
		uint32_t m_channels;
		uint32_t m_Width, m_Height;
		uint32_t m_rows;

		struct VkTextureData
		{
			VkSampler				sampler;
			VkImage					image;
			VkImageLayout			imageLayout;
			VkMemoryAllocateInfo	memoryAlloc;
			VkDeviceMemory			mem;
			VkImageView				view;
			VkDescriptorImageInfo	descsImgInfo;
		} m_textureData;
	};
}