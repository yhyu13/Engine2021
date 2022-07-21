#include "engine-precompiled-header.h"
#include "OpenGLTexture.h"
#include "engine/EngineEssential.h"
#include "engine/math/MathUtil.h"
#include "engine/core/utility/FSafe.h"

#include <stb_image.h>
#include <stb_image_write.h>
#include <glad/glad.h>
#include <SOIL2.h>

namespace longmarch {
	OpenGLTexture2D::OpenGLTexture2D(Texture::Setting data)
		:
		m_Width(data.width),
		m_Height(data.height),
		m_float_type(data.float_type),
		m_channels(data.channels),
		m_rows(data.rows)
	{
		/**************************************************************
		*	Calculate mipmap levels
		*	Reference: https://stackoverflow.com/questions/9572414/how-many-mipmaps-does-a-texture-have-in-opengl
		*	https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/7
		**************************************************************/
		if (data.has_mipmap)
		{
			m_max_level = (int)glm::floor(glm::log2(float(MAX(m_Width, m_Height))));
			if (data.mipmap_level > 0)
			{
				m_max_level = MIN(m_max_level, data.mipmap_level);
			}
		}
		else
		{
			m_max_level = 0;
		}
		if (m_channels == 4)
		{
			m_InternalFormat = (m_float_type) ? GL_RGBA16F : GL_RGBA8;
			m_DataFormat = GL_RGBA;
		}
		else if (m_channels == 3)
		{
			m_InternalFormat = (m_float_type) ? GL_RGB16F : GL_RGB8;
			m_DataFormat = GL_RGB;
		}
		else if (m_channels == 2)
		{
			m_InternalFormat = (m_float_type) ? GL_RG16F : GL_RG8;
			m_DataFormat = GL_RG;
		}
		else if (m_channels == 1)
		{
			m_InternalFormat = (m_float_type) ? GL_R16F : GL_R8;
			m_DataFormat = GL_RED;
		}
		else
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Invalid channel for texture: " + wStr(m_channels) + L"!");
		}

		// Initialize to black image
		void* _data = nullptr;
		if (!data.input_data)
		{
			if (m_float_type)
			{
				_data = new float[m_Width * m_Height * m_channels];
				memset(_data, 0, m_Width * m_Height * m_channels * sizeof(float));
			}
			else
			{
				_data = new uint8_t[m_Width * m_Height * m_channels];
				memset(_data, 0, m_Width * m_Height * m_channels * sizeof(uint8_t));
			}
		}

		glGenTextures(1, &m_RenderTargetID);
		glBindTexture(GL_TEXTURE_2D, m_RenderTargetID);
		glTextureStorage2D(m_RenderTargetID, m_max_level + 1, m_InternalFormat, m_Width, m_Height);
		glTextureSubImage2D(m_RenderTargetID, 0, 0, 0, m_Width, m_Height, m_DataFormat, (m_float_type) ? GL_FLOAT : GL_UNSIGNED_BYTE, (_data) ? _data : data.input_data);

		// 4x anisotropy filter by default
		glTextureParameterf(m_RenderTargetID, GL_TEXTURE_MAX_ANISOTROPY, 4.0f);
		// default repeat wrap method
		glTextureParameteri(m_RenderTargetID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RenderTargetID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		if (data.has_mipmap)
		{
			// if has mipmap, generate the mipmap
			glTextureParameteri(m_RenderTargetID, GL_TEXTURE_MAG_FILTER, (data.linear_filter) ? GL_LINEAR : GL_NEAREST);
			glTextureParameteri(m_RenderTargetID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			glTextureParameteri(m_RenderTargetID, GL_TEXTURE_MAG_FILTER, (data.linear_filter) ? GL_LINEAR : GL_NEAREST);
			glTextureParameteri(m_RenderTargetID, GL_TEXTURE_MIN_FILTER, (data.linear_filter) ? GL_LINEAR : GL_NEAREST);
		}
		GLCHECKERROR;
		if (_data)
		{
			delete[] _data;
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(const fs::path& path)
	{
		auto _path = path.string();
		int width, height, channels, num_levels;

		auto file_extension = path.extension();
		if (file_extension.compare(".hdr") == 0
			|| file_extension.compare(".rgbe") == 0)
		{
			stbi_set_flip_vertically_on_load_thread(1);
			float* data = stbi_loadf(_path.c_str(), &width, &height, &channels, 0);
			ASSERT(data, "Failed to load image :" + _path + "!");
			m_Width = width;
			m_Height = height;
			m_float_type = true;
			m_max_level = (int)glm::floor(glm::log2(float(MAX(width, height))));
			m_channels = channels;
			if (m_channels == 4)
			{
				m_InternalFormat = GL_RGBA16F;
				m_DataFormat = GL_RGBA;
			}
			else if (m_channels == 3)
			{
				m_InternalFormat = GL_RGB16F;
				m_DataFormat = GL_RGB;
			}
			else if (m_channels == 2)
			{
				m_InternalFormat = GL_RG16F;
				m_DataFormat = GL_RG;
			}
			else if (m_channels == 1)
			{
				m_InternalFormat = GL_R16F;
				m_DataFormat = GL_R;
			}
			else
			{
				throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Invalid channel for texture: " + wStr(m_channels) + L"!");
			}
			glGenTextures(1, &m_RenderTargetID);
			glBindTexture(GL_TEXTURE_2D, m_RenderTargetID);
			// Byte alignment
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTextureStorage2D(m_RenderTargetID, m_max_level + 1, m_InternalFormat, m_Width, m_Height);
			glTextureSubImage2D(m_RenderTargetID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_FLOAT, data);
			// 4x anisotropy filter by default
			glTextureParameterf(m_RenderTargetID, GL_TEXTURE_MAX_ANISOTROPY, 4.0f);
			// default repeat wrap method
			glTextureParameteri(m_RenderTargetID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_RenderTargetID, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTextureParameteri(m_RenderTargetID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_RenderTargetID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
			stbi_image_free(data);
		}
		else if (file_extension.compare(".dds") == 0)
		{
			m_RenderTargetID = SOIL_load_OGL_texture(_path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_DDS_LOAD_DIRECT);
			auto error = SOIL_last_result();
			ENGINE_EXCEPT_IF(m_RenderTargetID == 0, L"Texture loading fails at : " + wStr(_path) + L" with error : " + wStr(error));
			m_float_type = false;
		}
		else
		{
			stbi_set_flip_vertically_on_load_thread(1);
			stbi_uc* data = stbi_load(_path.c_str(), &width, &height, &channels, 0);
			ASSERT(data, "Failed to load image :" + _path + "!");
			m_Width = width;
			m_Height = height;
			m_float_type = false;
			m_max_level = (int)glm::floor(glm::log2(float(MAX(width, height))));
			m_channels = channels;
			if (m_channels == 4)
			{
				m_InternalFormat = GL_RGBA8;
				m_DataFormat = GL_RGBA;
			}
			else if (m_channels == 3)
			{
				m_InternalFormat = GL_RGB8;
				m_DataFormat = GL_RGB;
			}
			else if (m_channels == 2)
			{
				m_InternalFormat = GL_RG8;
				m_DataFormat = GL_RG;
			}
			else if (m_channels == 1)
			{
				m_InternalFormat = GL_R8;
				m_DataFormat = GL_R;
			}
			else
			{
				throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Invalid channel for texture: " + wStr(m_channels) + L"!");
			}
			glGenTextures(1, &m_RenderTargetID);
			glBindTexture(GL_TEXTURE_2D, m_RenderTargetID);
			// Byte alignment
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTextureStorage2D(m_RenderTargetID, m_max_level + 1, m_InternalFormat, m_Width, m_Height);
			glTextureSubImage2D(m_RenderTargetID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
			// 4x anisotropy filter by default
			glTextureParameterf(m_RenderTargetID, GL_TEXTURE_MAX_ANISOTROPY, 4.0f);
			// default repeat wrap method
			glTextureParameteri(m_RenderTargetID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_RenderTargetID, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTextureParameteri(m_RenderTargetID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_RenderTargetID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
			stbi_image_free(data);
		}
		GLCHECKERROR;
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RenderTargetID);
	}

	void OpenGLTexture2D::AttachToFrameBuffer() const
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_RenderTargetID, 0);
		GLCHECKERROR;
	}

	void OpenGLTexture2D::BindTexture(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RenderTargetID);
		GLCHECKERROR;
	}

	void OpenGLTexture2D::WriteToPNG(const fs::path& path) const
	{
		auto _path = path.string();
		if (IsFloatType())
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Trying to write a float type texture to PNG file format!");
		}
		uint32_t   output_width, output_height;

		output_width = m_Width;
		output_height = m_Height;

		/// READ THE PIXELS VALUES from FBO AND SAVE TO A .HDR FILE
		uint8_t* pixels = (uint8_t*)malloc(output_width * output_height * m_channels * sizeof(uint8_t));

		/// READ THE CONTENT FROM THE FBO
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(0, 0, output_width, output_height, m_DataFormat, GL_UNSIGNED_BYTE, pixels);
		GLCHECKERROR;

		// yuhang : Double check we file path is valid
		FILE* output_image = nullptr;
		FOpenS(output_image, _path.c_str(), "wt");
		if (!output_image)
		{
			EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Fail to open file location: " + wStr(_path) + L"!"));
		}
		PRINT("Writing .PNG to : " + _path);
		PRINT("width: " + Str(output_width) + " Height: " + Str(output_height) + " Channel: " + Str(m_channels));
		fclose(output_image);
		
		int result;
		{
			atomic_flag_guard lock(stbi_png_write_lock);
			stbi_flip_vertically_on_write(1);
			result = stbi_write_png(_path.c_str(), output_width, output_height, m_channels, pixels, output_width * m_channels);
		}
		if (result == 0)
		{
			EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Fail to wirte to file location: " + wStr(_path) + L"!"));
		}
		free(pixels);
	}

	void OpenGLTexture2D::WriteToHDR(const fs::path& path) const
	{
		auto _path = path.string();
		if (!IsFloatType())
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Trying to write a non-float type texture to HDR file format!");
		}
		uint32_t   output_width, output_height;

		output_width = m_Width;
		output_height = m_Height;

		/// READ THE PIXELS VALUES from FBO AND SAVE TO A .HDR FILE
		float* pixels = (float*)malloc(output_width * output_height * m_channels * sizeof(float));

		/// READ THE CONTENT FROM THE FBO
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(0, 0, output_width, output_height, m_DataFormat, GL_FLOAT, pixels);
		GLCHECKERROR;

		// yuhang : Double check we file path is valid
		FILE* output_image = nullptr;
		FOpenS(output_image, _path.c_str(), "wt");
		if (!output_image)
		{
			EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Fail to open file location: " + wStr(_path) + L"!"));
		}
		PRINT("Writing .HDR to : " + _path);
		PRINT("width: " + Str(output_width) + " Height: " + Str(output_height) + " Channel: " + Str(m_channels));
		fclose(output_image);
		
		int result;
		{
			atomic_flag_guard lock(stbi_hdr_write_lock);
			stbi_flip_vertically_on_write(1);
			result = stbi_write_hdr(_path.c_str(), output_width, output_height, m_channels, pixels);
		}
		if (result == 0)
		{
			EngineException::Push(EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Fail to wirte to file location: " + wStr(_path) + L"!"));
		}
		free(pixels);
	}

	void OpenGLTexture2D::ReadTexture(void* tex) const
	{
		/// READ THE CONTENT FROM THE FBO
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(0, 0, m_Width, m_Height, m_DataFormat, IsFloatType() ? GL_FLOAT : GL_UNSIGNED_BYTE, tex);
		GLCHECKERROR;
	}
}
