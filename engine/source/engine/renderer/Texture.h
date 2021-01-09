#pragma once

#include "engine/core/EngineCore.h"

namespace longmarch {
	class ENGINE_API Texture
	{
	public:
		struct Setting
		{
			uint32_t width = 1;
			uint32_t height = 1;
			uint32_t channels = 4;
			uint32_t mipmap_level = 0;
			bool has_mipmap = false;
			bool linear_filter = false;
			bool float_type = false;
			void* input_data = nullptr;
			int rows = 1;
		};

		virtual ~Texture() = default;

		virtual bool IsFloatType() const = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual uint32_t GetMaxMipMapLevel() const = 0;
		virtual uint32_t GetTextureRowCount() const = 0;

		virtual void BindTexture(uint32_t slot) const = 0;

		//! Methods below are useful to attach a texture to a framebuffer, then read the rendered result back
		//! Always Bind FBO
		virtual void AttachToFrameBuffer() const = 0;
		//! Always Bind FBO
		virtual void WriteToPNG(const fs::path& path) const = 0;
		//! Always Bind FBO
		virtual void WriteToHDR(const fs::path& path) const = 0;
		//! Always Bind FBO
		virtual void ReadTexture(void* tex) const = 0;
	};

	class ENGINE_API Texture2D : public Texture
	{
	public:
		static std::shared_ptr<Texture2D> Create(Texture::Setting data);
		static std::shared_ptr<Texture2D> LoadFromFile(const fs::path& path);
	};
}