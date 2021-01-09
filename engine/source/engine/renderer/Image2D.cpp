#include "engine-precompiled-header.h"
#include "Image2D.h"
#include <stb_image.h>

longmarch::Image2D::Image2D()
	:
	m_Width(0),
	m_Height(0),
	m_Channels(4),
	m_float_type(false),
	m_data(nullptr)
{
}

longmarch::Image2D::~Image2D()
{
	if (m_data)
	{
		free(m_data);
	}
}

longmarch::Image2D::Image2D(Image2D::Setting data)
	:
	m_Width(data.width),
	m_Height(data.height),
	m_Channels(data.channels),
	m_float_type(data.float_type),
	m_data(data.data),
	m_rows(data.rows)
{
	size_t bytePerPixel = IsFloatType() ? sizeof(float) : sizeof(uint8_t);
	bytePerPixel *= m_Channels;
	size_t totalByte = m_Width * m_Height * bytePerPixel;
	if (data.data)
	{
		m_data = malloc(totalByte);
		memcpy_s(m_data, totalByte, data.data, totalByte);
	}
	else
	{
		m_data = malloc(totalByte);
		memset(m_data, 0, totalByte);
	}
}

longmarch::Image2D::Image2D(const Image2D& other)
{
	ASSERT(this != &other, "Copy construtor the same object is not allowed!");
	{
		m_Width = other.m_Width;
		m_Height = other.m_Height;
		m_Channels = other.m_Channels;
		m_float_type = other.m_float_type;
		m_rows = other.m_rows;
		size_t bytePerPixel = IsFloatType() ? sizeof(float) : sizeof(uint8_t);
		bytePerPixel *= m_Channels;
		size_t totalByte = m_Width * m_Height * bytePerPixel;
		m_data = malloc(totalByte);
		memcpy_s(m_data, totalByte, other.m_data, totalByte);
	}
}

std::shared_ptr<longmarch::Image2D> longmarch::Image2D::LoadFromFile(const fs::path& path)
{
	std::shared_ptr<longmarch::Image2D> img = MemoryManager::Make_shared<longmarch::Image2D>();
	int width, height, channels;
	stbi_set_flip_vertically_on_load_thread(1);
	auto file_extension = path.extension();
	if (file_extension.compare(".hdr") == 0
		|| file_extension.compare(".rgbe") == 0)
	{
		float* data = stbi_loadf(path.string().c_str(), &width, &height, &channels, 0);
		ASSERT(data, "Failed to load image :" + path.string() + "!");
		img->m_Channels = channels;
		img->m_Width = width;
		img->m_Height = height;
		img->m_data = data;
		img->m_float_type = true;
	}
	else if (file_extension.compare(".dds") == 0)
	{
		throw NotImplementedException();
	}
	else
	{
		stbi_uc* data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
		ASSERT(data, "Failed to load image :" + path.string() + "!");
		img->m_Channels = channels;
		img->m_Width = width;
		img->m_Height = height;
		img->m_data = data;
		img->m_float_type = false;
	}
	return img;
}

uint32_t longmarch::Image2D::GetWidth() const
{
	return m_Width;
}

uint32_t longmarch::Image2D::GetHeight() const
{
	return m_Height;
}

uint32_t longmarch::Image2D::GetChannels() const
{
	return m_Channels;
}

bool longmarch::Image2D::IsFloatType() const
{
	return m_float_type;
}

void* longmarch::Image2D::GetData() const
{
	return m_data;
}

Vec4f longmarch::Image2D::GetPixel(uint32_t row, uint32_t col) const
{
	ASSERT(row < m_Width, Str(row) + " is out of range of size " + Str(m_Width) + "!");
	ASSERT(col < m_Height, Str(col) + " is out of range of size " + Str(m_Height) + "!");

	Vec4f rgba;
	size_t bytePerPixel = IsFloatType() ? sizeof(float) : sizeof(uint8_t);
	bytePerPixel *= m_Channels;

	void* pixelOffset = (uint8_t*)m_data + (row + m_Width * col) * bytePerPixel;
	if (IsFloatType())
	{
		rgba.r = ((float*)pixelOffset)[0];
		rgba.g = ((float*)pixelOffset)[1];
		rgba.b = ((float*)pixelOffset)[2];
		rgba.a = (m_Channels == 3) ? 1 : ((float*)pixelOffset)[3];
	}
	else
	{
		rgba.r = ((uint8_t*)pixelOffset)[0] / 255.0;
		rgba.g = ((uint8_t*)pixelOffset)[1] / 255.0;
		rgba.b = ((uint8_t*)pixelOffset)[2] / 255.0;
		rgba.a = (m_Channels == 3) ? 1 : ((uint8_t*)pixelOffset)[3] / 255.0;
	}
	return rgba;
}

void longmarch::Image2D::WritePixel(uint32_t row, uint32_t col, const Vec4f& rgba)
{
	ASSERT(row < m_Width, Str(row) + " is out of range of size " + Str(m_Width) + "!");
	ASSERT(col < m_Height, Str(col) + " is out of range of size " + Str(m_Height) + "!");

	size_t bytePerPixel = IsFloatType() ? sizeof(float) : sizeof(uint8_t);
	bytePerPixel *= m_Channels;

	void* pixelOffset = (uint8_t*)m_data + (row + m_Width * col) * bytePerPixel;
	if (IsFloatType())
	{
		((float*)pixelOffset)[0] = MAX(rgba.r, 0.f);
		((float*)pixelOffset)[1] = MAX(rgba.g, 0.f);
		((float*)pixelOffset)[2] = MAX(rgba.b, 0.f);
		if (m_Channels == 4)
		{
			((float*)pixelOffset)[3] = MAX(rgba.a, 0.f);
		}
	}
	else
	{
		((uint8_t*)pixelOffset)[0] = (uint8_t)glm::clamp(rgba.r * 255.0f, 0.0f, 255.0f);
		((uint8_t*)pixelOffset)[1] = (uint8_t)glm::clamp(rgba.g * 255.0f, 0.0f, 255.0f);
		((uint8_t*)pixelOffset)[2] = (uint8_t)glm::clamp(rgba.b * 255.0f, 0.0f, 255.0f);
		if (m_Channels == 4)
		{
			((uint8_t*)pixelOffset)[3] = (uint8_t)glm::clamp(rgba.a * 255.0f, 0.0f, 255.0f);
		}
	}
}

void longmarch::Image2D::WriteToPNG(const fs::path& _path) const
{
	auto path = _path.string();
	if (IsFloatType())
	{
		EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Trying to write a float type texture to PNG file format!"));
	}
	uint32_t   output_width, output_height;

	output_width = m_Width;
	output_height = m_Height;

	/// READ THE PIXELS VALUES from FBO AND SAVE TO A .HDR FILE
	uint8_t* pixels = (uint8_t*)m_data;

	FILE* output_image;
	output_image = fopen(path.c_str(), "wt");
	if (!output_image)
	{
		EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Fail to open file location: " + str2wstr(path) + L"!"));
	}
	PRINT("Writing .HDR to : " + path);
	PRINT("width: " + Str(output_width) + " Height: " + Str(output_height) + " Channel: " + Str(m_Channels));
	fclose(output_image);
	int result;
	{
		atomic_flag_guard lock(stbi_png_write_lock);
		stbi_flip_vertically_on_write(1);
		result = stbi_write_png(path.c_str(), output_width, output_height, m_Channels, pixels, output_width * m_Channels);
	}
	if (result == 0)
	{
		EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Fail to wirte to file location: " + str2wstr(path) + L"!"));
	}
}

void longmarch::Image2D::WriteToHDR(const fs::path& _path) const
{
	auto path = _path.string();
	if (!IsFloatType())
	{
		EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Trying to a non float type texture to HDR file format!"));
	}
	uint32_t   output_width, output_height;

	output_width = m_Width;
	output_height = m_Height;

	/// READ THE PIXELS VALUES from FBO AND SAVE TO A .HDR FILE
	float* pixels = (float*)m_data;

	FILE* output_image;
	output_image = fopen(path.c_str(), "wt");
	if (!output_image)
	{
		EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Fail to open file location: " + str2wstr(path) + L"!"));
	}
	PRINT("Writing .HDR to : " + path);
	PRINT("width: " + Str(output_width) + " Height: " + Str(output_height) + " Channel: " + Str(m_Channels));
	fclose(output_image);
	int result;
	{
		atomic_flag_guard lock(stbi_hdr_write_lock);
		stbi_flip_vertically_on_write(1);
		result = stbi_write_hdr(path.c_str(), output_width, output_height, m_Channels, pixels);
	}
	if (result == 0)
	{
		EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Fail to wirte to file location: " + str2wstr(path) + L"!"));
	}
}