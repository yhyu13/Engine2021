#pragma once
#include "engine/EngineEssential.h"
#include "engine/math/Geommath.h"

namespace AAAAgames
{
	class Image2D
	{
	public:
		struct Setting
		{
			uint32_t width = 0;
			uint32_t height = 0;
			uint32_t channels = 4;
			bool float_type = false;
			void* data = nullptr;
			uint32_t rows = 1;
		};

		Image2D();
		~Image2D();
		explicit Image2D(Image2D::Setting data);
		Image2D(const Image2D& other);

		static std::shared_ptr<Image2D> LoadFromFile(const fs::path& path);

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		uint32_t GetChannels() const;
		bool IsFloatType() const;
		void* GetData() const;
		Vec4f GetPixel(uint32_t row, uint32_t col) const;
		void WritePixel(uint32_t row, uint32_t col, const Vec4f& rgba);

		virtual void WriteToPNG(const fs::path& path) const;
		virtual void WriteToHDR(const fs::path& path) const;

	private:
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_Channels;
		bool m_float_type;
		void* m_data;
		uint32_t m_rows;
	};
}
