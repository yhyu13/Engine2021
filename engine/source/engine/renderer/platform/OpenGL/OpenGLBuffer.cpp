#include "engine-precompiled-header.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>
#include <cstdint>

#define SHADOW_FILTER GL_LINEAR

namespace longmarch {
	/**************************************************************
	*	Indexed Indirect Command Buffer
	**************************************************************/
	OpenGLIndexedIndirectCommandBuffer::OpenGLIndexedIndirectCommandBuffer(const void* data, size_t size)
	{
		if (data != nullptr)
		{
			ASSERT(size % GetElementSize() == 0, "OpenGLIndexedIndirectCommandBuffer must have integral multiples of IndexedIndirectCommandBuffer!");
			m_Count = size / GetElementSize();
			m_Capcity = size;
		}
		else
		{
			m_Count = 0;
			m_Capcity = size;
		}
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_RendererID);
		glBufferData(GL_DRAW_INDIRECT_BUFFER, size, data, GL_DYNAMIC_DRAW);
		GLCHECKERROR;
	}
	OpenGLIndexedIndirectCommandBuffer::~OpenGLIndexedIndirectCommandBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}
	void OpenGLIndexedIndirectCommandBuffer::Bind() const
	{
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_RendererID);
		GLCHECKERROR;
	}
	void OpenGLIndexedIndirectCommandBuffer::Unbind() const
	{
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
		GLCHECKERROR;
	}
	void OpenGLIndexedIndirectCommandBuffer::UpdateBufferData(const void* data, size_t size)
	{
		if (data != nullptr)
		{
			ASSERT(size % GetElementSize() == 0, "OpenGLIndexedIndirectCommandBuffer must have integral multiples of DrawIndexedIndirectCommand!");
			m_Count = size / GetElementSize();
			m_Capcity = size;
		}
		else
		{
			m_Count = 0;
			m_Capcity = size;
		}
		glNamedBufferData(m_RendererID, size, data, GL_DYNAMIC_DRAW);
		GLCHECKERROR;
	}
	/**************************************************************
	*	Vertex Buffer
	**************************************************************/
	OpenGLVertexBuffer::OpenGLVertexBuffer(const void* data, size_t size)
	{
		if (data != nullptr)
		{
			ASSERT(size % GetElementSize() == 0, "OpenGLVertexBuffer must have integral multiples of float!");
			m_Count = size / GetElementSize();
			m_Capcity = size;
		}
		else
		{
			m_Count = 0;
			m_Capcity = size;
		}
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
		GLCHECKERROR;
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		GLCHECKERROR;
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		GLCHECKERROR;
	}

	void OpenGLVertexBuffer::AppendBufferData(const std::shared_ptr<VertexBuffer>& read_buffer)
	{
		CopyBufferData(m_Count * GetElementSize(), read_buffer, 0);
		m_Count += read_buffer->GetCount();
	}

	void OpenGLVertexBuffer::CopyBufferData(size_t write_offset, const std::shared_ptr<VertexBuffer>& read_buffer, size_t read_offset)
	{
		glCopyNamedBufferSubData(read_buffer->GetRendererID(), m_RendererID, read_offset, write_offset, read_buffer->GetCount() * read_buffer->GetElementSize() - read_offset);
		GLCHECKERROR;
	}

	void OpenGLVertexBuffer::AppendBufferData(const void* data, size_t size)
	{
		UpdateBufferSubData(data, size, m_Count * GetElementSize());
		ASSERT(size % GetElementSize() == 0, "OpenGLVertexBuffer must have integral multiples of float!");
		m_Count += size / GetElementSize();
	}

	void OpenGLVertexBuffer::UpdateBufferSubData(const void* data, size_t size, size_t write_offset)
	{
		glNamedBufferSubData(m_RendererID, write_offset, size, data);
		GLCHECKERROR;
	}

	void OpenGLVertexBuffer::UpdateBufferData(const void* data, size_t size)
	{
		if (data != nullptr) [[likely]]
		{
			ASSERT(size % GetElementSize() == 0, "OpenGLVertexBuffer must have integral multiples of float!");
			m_Count = size / GetElementSize();
			m_Capcity = size;
		}
		else [[unlikely]]
		{
			m_Count = 0;
			m_Capcity = size;
		}
		glNamedBufferData(m_RendererID, size, data, GL_DYNAMIC_DRAW);
		GLCHECKERROR;
	}
	/**************************************************************
	*	Index Buffer
	**************************************************************/
	OpenGLIndexBuffer::OpenGLIndexBuffer(const void* data, size_t size, size_t elementSize)
		: m_Count(size)
	{
		SetElementSize(elementSize);
		if (data != nullptr)
		{
			ASSERT(size % GetElementSize() == 0, "OpenGLIndexBuffer must have integral multiples of uint32_t!");
			m_Count = size / GetElementSize();
			m_Capcity = size;
		}
		else
		{
			m_Count = 0;
			m_Capcity = size;
		}
		//OpenGL 4.5
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
		GLCHECKERROR;
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		GLCHECKERROR;
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		GLCHECKERROR;
	}

	void OpenGLIndexBuffer::AppendBufferData(const std::shared_ptr<IndexBuffer>& read_buffer)
	{
		CopyBufferData(m_Count * GetElementSize(), read_buffer, 0);
		m_Count += read_buffer->GetCount();
	}

	void OpenGLIndexBuffer::CopyBufferData(size_t write_offset, const std::shared_ptr<IndexBuffer>& read_buffer, size_t read_offset)
	{
		glCopyNamedBufferSubData(read_buffer->GetRendererID(), m_RendererID, read_offset, write_offset, read_buffer->GetCount() * read_buffer->GetElementSize() - read_offset);
		GLCHECKERROR;
	}

	void OpenGLIndexBuffer::AppendBufferData(const void* data, size_t size)
	{
		UpdateBufferSubData(data, size, m_Count * GetElementSize());
		ASSERT(size % GetElementSize() == 0, "OpenGLIndexBuffer must have integral multiples of uint32_t!");
		m_Count += size / GetElementSize();
	}

	void OpenGLIndexBuffer::UpdateBufferSubData(const void* data, size_t size, size_t write_offset)
	{
		glNamedBufferSubData(m_RendererID, write_offset, size, data);
		GLCHECKERROR;
	}

	void OpenGLIndexBuffer::UpdateBufferData(const void* data, size_t size)
	{
		if (data != nullptr) [[likely]]
		{
			ASSERT(size % GetElementSize() == 0, "OpenGLIndexBuffer must have integral multiples of uint32_t!");
			m_Count = size / GetElementSize();
			m_Capcity = size;
		}
		else [[unlikely]]
		{
			m_Count = 0;
			m_Capcity = size;
		}
		glNamedBufferData(m_RendererID, size, data, GL_DYNAMIC_DRAW);
		GLCHECKERROR;
	}
	/**************************************************************
	*	Uniform Buffer
	**************************************************************/
	OpenGLUniformBuffer::OpenGLUniformBuffer(const void* data, size_t size)
	{
		//OpenGL 4.5
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
		GLCHECKERROR;
	}
	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}
	void OpenGLUniformBuffer::Bind(uint32_t slot) const
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, slot, m_RendererID);
		GLCHECKERROR;
	}
	void OpenGLUniformBuffer::Unbind() const
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
		GLCHECKERROR;
	}
	void OpenGLUniformBuffer::UpdateBufferData(const void* data, size_t size)
	{
		glNamedBufferData(m_RendererID, size, data, GL_DYNAMIC_DRAW);
		GLCHECKERROR;
	}
	void* OpenGLUniformBuffer::GetUniformBufferMapping(size_t size, size_t offset)
	{
		/*
			Reference: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glMapBufferRange.xhtml
		*/
		void* ret = glMapBufferRange(GL_UNIFORM_BUFFER, offset, size, GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT);// | GL_MAP_INVALIDATE_BUFFER_BIT);
		GLCHECKERROR;
		return ret;
	}
	void OpenGLUniformBuffer::EndUniformBufferMapping()
	{
		glUnmapBuffer(GL_UNIFORM_BUFFER);
		GLCHECKERROR;
	}

	/**************************************************************
	*	Shader Storage Buffer
	**************************************************************/
	OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(const void* data, size_t size)
	{
		//OpenGL 4.5
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
		GLCHECKERROR;
	}
	OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}
	void OpenGLShaderStorageBuffer::Bind(uint32_t slot) const
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, m_RendererID);
		GLCHECKERROR;
	}
	void OpenGLShaderStorageBuffer::Unbind() const
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		GLCHECKERROR;
	}
	void OpenGLShaderStorageBuffer::UpdateBufferData(const void* data, size_t size)
	{
		glNamedBufferData(m_RendererID, size, data, GL_DYNAMIC_DRAW);
		GLCHECKERROR;
	}
	void* OpenGLShaderStorageBuffer::GetShaderStorageBufferMapping(size_t size, size_t offset)
	{
		/*
			Reference: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glMapBufferRange.xhtml
		*/
		void* ret = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, size, GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT);// | GL_MAP_INVALIDATE_BUFFER_BIT);
		GLCHECKERROR;
		return ret;
	}
	void OpenGLShaderStorageBuffer::EndShaderStorageBufferMapping()
	{
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		GLCHECKERROR;
	}

	/**************************************************************
	*	Frame Buffer
	**************************************************************/
	OpenGLFrameBuffer::OpenGLFrameBuffer(uint32_t width, uint32_t height, FrameBuffer::BUFFER_FORMAT type)
	{
		m_width = width;
		m_height = height;
		m_format = type;

		uint32_t format;
		uint32_t channel;
		uint32_t data_type;
		switch (type)
		{
		case longmarch::FrameBuffer::BUFFER_FORMAT::Uint8:
			format = GL_RGBA8;
			channel = GL_RGBA;
			data_type = GL_UNSIGNED_BYTE;
			break;
		case longmarch::FrameBuffer::BUFFER_FORMAT::Float16:
			format = GL_RGBA16F;
			channel = GL_RGBA;
			data_type = GL_FLOAT;
			break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Unknown FrameBuffer::BUFFER_FORMAT type!");
		}

		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		//Create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		glGenRenderbuffers(1, &m_DepthID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_DepthID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthID);

		// Create a ture and attach FBO's color 0 attachment.  The
		// GL_RGBA32F and GL_RGBA constants set this ture to be 32 bit
		// floats for each of the 4 components.  Many other choices are
		// possible.
		glGenTextures(1, &m_RenderTargetID);
		glBindTexture(GL_TEXTURE_2D, m_RenderTargetID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, channel, data_type, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_RenderTargetID, 0);
		glGenerateMipmap(GL_TEXTURE_2D);

		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

		// Check for completeness/correctness
		int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != int(GL_FRAMEBUFFER_COMPLETE))
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"FBO Error : " + wStr(status));
		}

		// Unbind the fbo until it's ready to be used
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_RenderTargetID);
		glDeleteRenderbuffers(1, &m_DepthID);
	}
	void OpenGLFrameBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		GLCHECKERROR;
	}
	void OpenGLFrameBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	void OpenGLFrameBuffer::BindTexture(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RenderTargetID);
		GLCHECKERROR;
	}

	void OpenGLFrameBuffer::GenerateMipmaps() const
	{
		glBindTexture(GL_TEXTURE_2D, m_RenderTargetID);
		glGenerateMipmap(GL_TEXTURE_2D);
		GLCHECKERROR;
	}

	/**************************************************************
	*	Shadow Buffer
	**************************************************************/
	OpenGLShadowBuffer::OpenGLShadowBuffer(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;
		m_type = ShadowBuffer::SHADOW_MAP_TYPE::BASIC;

		GLCHECKERROR;
		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		// Even though we can simply write depth component into 2D texture, but since we used reverse Z, we need to alter the z value in shader.
		// So we use a depth render buffer, together with a color texture that stores the depth value.

		glGenRenderbuffers(1, &m_DepthID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_DepthID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthID);

		glGenTextures(1, &m_RenderTargetID);
		glBindTexture(GL_TEXTURE_2D, m_RenderTargetID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, SHADOW_FILTER); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, SHADOW_FILTER); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		float color[] = { 0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &color[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_FLOAT, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_RenderTargetID, 0);

		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

		// Check for completeness/correctness
		int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != int(GL_FRAMEBUFFER_COMPLETE))
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"FBO Error : " + wStr(status));
		}
		// Unbind the fbo until it's ready to be used
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}

	OpenGLShadowBuffer::~OpenGLShadowBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_RenderTargetID);
		glDeleteRenderbuffers(1, &m_DepthID);
	}

	void OpenGLShadowBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		GLCHECKERROR;
	}

	void OpenGLShadowBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}

	void OpenGLShadowBuffer::BindTexture(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RenderTargetID);
		GLCHECKERROR;
	}
	/**************************************************************
	*	Compare Shadow Buffer
	**************************************************************/
	OpenGLCompareShadowBuffer::OpenGLCompareShadowBuffer(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;
		m_type = ShadowBuffer::SHADOW_MAP_TYPE::BASIC_COMPARE;

		GLCHECKERROR;
		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);		

		// Even though we can simply write depth component into 2D texture, but since we used reverse Z, we need to alter the z value in shader.
		// So we use a depth render buffer, together with a color texture that stores the depth value.
		
		glGenRenderbuffers(1, &m_DepthID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_DepthID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthID);

		glGenTextures(1, &m_RenderTargetID);
		glBindTexture(GL_TEXTURE_2D, m_RenderTargetID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, SHADOW_FILTER); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, SHADOW_FILTER); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
		float color[] = { 0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &color[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_RenderTargetID, 0);

		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

		// Check for completeness/correctness
		int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != int(GL_FRAMEBUFFER_COMPLETE))
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"FBO Error : " + wStr(status));
		}
		// Unbind the fbo until it's ready to be used
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	OpenGLCompareShadowBuffer::~OpenGLCompareShadowBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_RenderTargetID);
		glDeleteRenderbuffers(1, &m_DepthID);
	}
	void OpenGLCompareShadowBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		GLCHECKERROR;
	}
	void OpenGLCompareShadowBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	void OpenGLCompareShadowBuffer::BindTexture(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RenderTargetID);
		GLCHECKERROR;
	}
	/**************************************************************
	*	Moment Shadow Buffer
	**************************************************************/
	OpenGLMSMShadowBuffer::OpenGLMSMShadowBuffer(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;
		m_type = ShadowBuffer::SHADOW_MAP_TYPE::MOMENT4;

		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		//Create a renderbuffer object for depth attachment (we won't be sampling these)
		glGenRenderbuffers(1, &m_DepthID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_DepthID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthID);

		// Create a texture and attach FBO's color 0 attachment.  The
		// GL_RGBA32F and GL_RGBA constants set this texture to be 32 bit
		// floats for each of the 4 components.  Many other choices are
		// possible.
		glGenTextures(1, &m_RenderTargetID);
		glBindTexture(GL_TEXTURE_2D, m_RenderTargetID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, SHADOW_FILTER); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, SHADOW_FILTER); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		float color[] = { 0,0,0,0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &color[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_RenderTargetID, 0);

		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

		// Check for completeness/correctness
		int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != int(GL_FRAMEBUFFER_COMPLETE))
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"FBO Error : " + wStr(status));
		}

		// Unbind the fbo until it's ready to be used
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	OpenGLMSMShadowBuffer::~OpenGLMSMShadowBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteRenderbuffers(1, &m_DepthID);
		glDeleteTextures(1, &m_RenderTargetID);
	}
	void OpenGLMSMShadowBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		GLCHECKERROR;
	}
	void OpenGLMSMShadowBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	void OpenGLMSMShadowBuffer::BindTexture(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RenderTargetID);
		GLCHECKERROR;
	}
	/**************************************************************
	*	Cube Moment Shadow Buffer
	**************************************************************/
	OpenGLMSMCubeShadowBuffer::OpenGLMSMCubeShadowBuffer(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;
		m_type = ShadowBuffer::SHADOW_MAP_TYPE::MOMENT4_CUBE;

		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		glGenTextures(1, &m_DepthID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_DepthID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, SHADOW_FILTER);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, SHADOW_FILTER);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		float color[] = { 0 };
		glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, &color[0]);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthID, 0);

		glGenTextures(1, &m_RenderTargetID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RenderTargetID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, SHADOW_FILTER); 
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, SHADOW_FILTER); 
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		float color2[] = { 0,0,0,0 };
		glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, &color2[0]);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		}
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_RenderTargetID, 0);

		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

		// Check for completeness/correctness
		int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != int(GL_FRAMEBUFFER_COMPLETE))
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"FBO Error : " + wStr(status));
		}

		// Unbind the fbo until it's ready to be used
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	OpenGLMSMCubeShadowBuffer::~OpenGLMSMCubeShadowBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_DepthID);
		glDeleteTextures(1, &m_RenderTargetID);
	}
	void OpenGLMSMCubeShadowBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		GLCHECKERROR;
	}
	void OpenGLMSMCubeShadowBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	void OpenGLMSMCubeShadowBuffer::BindTexture(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RenderTargetID);
		GLCHECKERROR;
	}
	/**************************************************************
	*	OpenGLShadowArrayBuffer Buffer
	**************************************************************/
	OpenGLShadowArrayBuffer::OpenGLShadowArrayBuffer(uint32_t width, uint32_t height, uint32_t depth)
	{
		m_width = width;
		m_height = height;
		m_depth = depth;
		m_type = ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_BASIC;

		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		
		// We can't simply write depth component into array texture, so we use a depth render buffer (cleared and used for each layer)
		// together with a color texture that stores the depth value.

		//Create a renderbuffer object for depth attachment (we won't be sampling these)
		glGenRenderbuffers(1, &m_DepthID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_DepthID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthID);

		glGenTextures(1, &m_RenderTargetID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RenderTargetID);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, SHADOW_FILTER); 
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, SHADOW_FILTER); 
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R16F, width, height, depth, 0, GL_RED, GL_FLOAT, NULL);

		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

		// Check for completeness/correctness
		int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != int(GL_FRAMEBUFFER_COMPLETE))
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"FBO Error : " + wStr(status));
		}

		// Unbind the fbo until it's ready to be used
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	OpenGLShadowArrayBuffer::~OpenGLShadowArrayBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteRenderbuffers(1, &m_DepthID);
		glDeleteTextures(1, &m_RenderTargetID);
	}
	void OpenGLShadowArrayBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		GLCHECKERROR;
	}
	void OpenGLShadowArrayBuffer::BindLayer(uint32_t slot) const
	{
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_RenderTargetID, 0, slot);
		GLCHECKERROR;
	}
	void OpenGLShadowArrayBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	void OpenGLShadowArrayBuffer::BindTexture(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RenderTargetID);
		GLCHECKERROR;
	}
	/**************************************************************
	*	OpenGLMSMShadowArrayBuffer Buffer
	**************************************************************/
	OpenGLCompareShadowArrayBuffer::OpenGLCompareShadowArrayBuffer(uint32_t width, uint32_t height, uint32_t depth)
	{
		m_width = width;
		m_height = height;
		m_depth = depth;
		m_type = ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_COMPARE;

		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		// We can't simply write depth component into array texture, so we use a depth render buffer (cleared and used for each layer)
		// together with a color texture that stores the depth value.

		//Create a renderbuffer object for depth attachment (we won't be sampling these)
		glGenRenderbuffers(1, &m_DepthID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_DepthID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthID);

		glGenTextures(1, &m_RenderTargetID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RenderTargetID);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, SHADOW_FILTER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, SHADOW_FILTER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R32F, width, height, depth, 0, GL_RED, GL_FLOAT, NULL);

		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

		// Check for completeness/correctness
		int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != int(GL_FRAMEBUFFER_COMPLETE))
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"FBO Error : " + wStr(status));
		}

		// Unbind the fbo until it's ready to be used
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	OpenGLCompareShadowArrayBuffer::~OpenGLCompareShadowArrayBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteRenderbuffers(1, &m_DepthID);
		glDeleteTextures(1, &m_RenderTargetID);
	}
	void OpenGLCompareShadowArrayBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		GLCHECKERROR;
	}
	void OpenGLCompareShadowArrayBuffer::BindLayer(uint32_t slot) const
	{
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_RenderTargetID, 0, slot);
		GLCHECKERROR;
	}
	void OpenGLCompareShadowArrayBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	void OpenGLCompareShadowArrayBuffer::BindTexture(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RenderTargetID);
		GLCHECKERROR;
	}
	/**************************************************************
	*	OpenGLMSMShadowArrayBuffer Buffer
	**************************************************************/
	OpenGLMSMShadowArrayBuffer::OpenGLMSMShadowArrayBuffer(uint32_t width, uint32_t height, uint32_t depth)
	{
		m_width = width;
		m_height = height;
		m_depth = depth;
		m_type = ShadowBuffer::SHADOW_MAP_TYPE::ARRAY_MOMENT4;

		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		//Create a renderbuffer object for depth attachment (we won't be sampling these)
		glGenRenderbuffers(1, &m_DepthID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_DepthID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthID);

		glGenTextures(1, &m_RenderTargetID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RenderTargetID);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, SHADOW_FILTER); 
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, SHADOW_FILTER); 
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		float color2[] = { 0,0,0,0 };
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, &color2[0]);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA16F, width, height, depth, 0, GL_RGBA, GL_FLOAT, NULL);

		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

		// Check for completeness/correctness
		int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != int(GL_FRAMEBUFFER_COMPLETE))
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"FBO Error : " + wStr(status));
		}
		// Unbind the fbo until it's ready to be used
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	OpenGLMSMShadowArrayBuffer::~OpenGLMSMShadowArrayBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteRenderbuffers(1, &m_DepthID);
		glDeleteTextures(1, &m_RenderTargetID);
	}
	void OpenGLMSMShadowArrayBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		GLCHECKERROR;
	}
	void OpenGLMSMShadowArrayBuffer::BindLayer(uint32_t slot) const
	{
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_RenderTargetID, 0, slot);
		GLCHECKERROR;
	}
	void OpenGLMSMShadowArrayBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	void OpenGLMSMShadowArrayBuffer::BindTexture(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RenderTargetID);
		GLCHECKERROR;
	}
	/**************************************************************
	*	SKyBox Buffer
	**************************************************************/
	OpenGLSkyBoxBuffer::OpenGLSkyBoxBuffer(uint32_t width, uint32_t height, SkyBoxBuffer::BUFFER_FORMAT type)
	{
		m_width = width;
		m_height = height;
		m_buffer_format = type;

		uint32_t format;
		switch (type)
		{
		case longmarch::SkyBoxBuffer::BUFFER_FORMAT::Float16:
			format = GL_RGB16F;
			break;
		case longmarch::SkyBoxBuffer::BUFFER_FORMAT::Float32:
			format = GL_RGB32F;
			break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Unknown SkyBoxBuffer::BUFFER_FORMAT type!");
		}
		/**************************************************************
		*	Calculate mipmap levels
		*	Reference: https://stackoverflow.com/questions/9572414/how-many-mipmaps-does-a-texture-have-in-opengl
		*	https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/7
		**************************************************************/
		m_max_level = (uint32_t)glm::floor(glm::log2(float((glm::max)(width, height))));
		if (m_max_level >= 5)
		{
			m_max_level = 5;
		}

		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		glGenTextures(1, &m_RenderTargetID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RenderTargetID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, m_max_level);

		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		float color[] = { 0,0,0,0 };
		glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, &color[0]);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_RenderTargetID, 0);

		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

		// Check for completeness/correctness
		int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != int(GL_FRAMEBUFFER_COMPLETE))
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"FBO Error : " + wStr(status));
		}

		// Unbind the fbo until it's ready to be used
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	OpenGLSkyBoxBuffer::~OpenGLSkyBoxBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_RenderTargetID);
	}
	void OpenGLSkyBoxBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		GLCHECKERROR;
	}
	void OpenGLSkyBoxBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	void OpenGLSkyBoxBuffer::BindTexture(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RenderTargetID);
		GLCHECKERROR;
	}
	void OpenGLSkyBoxBuffer::BindMipMap(uint32_t level) const
	{
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_RenderTargetID, level);
		GLCHECKERROR;
	}
	void OpenGLSkyBoxBuffer::GenerateMipmaps() const
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RenderTargetID);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		GLCHECKERROR;
	}
	/**************************************************************
	*	Geometry Buffer
	**************************************************************/
	OpenGLGBuffer::OpenGLGBuffer(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;

		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		glGenTextures(1, &m_DepthID);
		glBindTexture(GL_TEXTURE_2D, m_DepthID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthID, 0);

		// - normal buffer
		// - velocity buffer that stores texel sized velocity (i.e. screen space pixel velocity / screen size, i.e. clip space velocity on x-y plane)
		glGenTextures(1, &m_RenderNormal_VelocityID);
		glBindTexture(GL_TEXTURE_2D, m_RenderNormal_VelocityID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (GLuint)0, GL_TEXTURE_2D, m_RenderNormal_VelocityID, 0);

		// - color buffer with w as emissive that stores albedo color
		glGenTextures(1, &m_RenderAlbedo_EmssiveID);
		glBindTexture(GL_TEXTURE_2D, m_RenderAlbedo_EmssiveID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB10_A2, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (GLuint)1, GL_TEXTURE_2D, m_RenderAlbedo_EmssiveID, 0);

		// - Ambient occulusion buffer that stores baked AO texture
		// - Metallic buffer
		// - Roughness buffer
		glGenTextures(1, &m_RenderAO_Metallic_RoughnessID);
		glBindTexture(GL_TEXTURE_2D, m_RenderAO_Metallic_RoughnessID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (GLuint)2, GL_TEXTURE_2D, m_RenderAO_Metallic_RoughnessID, 0);

		// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
		GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 + (GLuint)0,
										GL_COLOR_ATTACHMENT0 + (GLuint)1,
										GL_COLOR_ATTACHMENT0 + (GLuint)2 };
		glDrawBuffers(sizeof(DrawBuffers) / sizeof(DrawBuffers[0]), DrawBuffers);

		// Check for completeness/correctness
		int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != int(GL_FRAMEBUFFER_COMPLETE))
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"FBO Error : " + wStr(status));
		}
		// Unbind the fbo until it's ready to be used
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	OpenGLGBuffer::~OpenGLGBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_DepthID);
		glDeleteTextures(1, &m_RenderNormal_VelocityID);
		glDeleteTextures(1, &m_RenderAlbedo_EmssiveID);
		glDeleteTextures(1, &m_RenderAO_Metallic_RoughnessID);
	}
	void OpenGLGBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		GLCHECKERROR;
	}
	void OpenGLGBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	void OpenGLGBuffer::BindTextures(const std::vector<GBUFFER_TEXTURE_TYPE>& texToBind, uint32_t offset) const
	{
		for (auto& tex : texToBind)
		{
			glBindTextureUnit((GLuint)tex + offset, GetTexutureID(tex));
			GLCHECKERROR;
		}
	}
	uint32_t OpenGLGBuffer::GetTexutureID(GBUFFER_TEXTURE_TYPE tex) const
	{
		switch (tex)
		{
		case longmarch::GBuffer::GBUFFER_TEXTURE_TYPE::DEPTH:
			return m_DepthID;
			break;
		case longmarch::GBuffer::GBUFFER_TEXTURE_TYPE::NORMAL_VELOCITY:
			return m_RenderNormal_VelocityID;
			break;
		case longmarch::GBuffer::GBUFFER_TEXTURE_TYPE::ALBEDO_EMSSIVE:
			return m_RenderAlbedo_EmssiveID;
			break;
		case longmarch::GBuffer::GBUFFER_TEXTURE_TYPE::BAKEDAO_METALLIC_ROUGHNESS:
			return m_RenderAO_Metallic_RoughnessID;
			break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Unknow GBuffer texture type!");
		}
	}
	/**************************************************************
	*	Think Geometry Buffer
	**************************************************************/
	OpenGLThinGBuffer::OpenGLThinGBuffer(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;

		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		glGenTextures(1, &m_DepthID);
		glBindTexture(GL_TEXTURE_2D, m_DepthID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		float color[] = { 0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &color[0]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthID, 0);

		// - normal buffer
		// - velocity buffer that stores texel sized velocity (i.e. screen space pixel velocity / screen size, i.e. clip space velocity on x-y plane)
		glGenTextures(1, &m_RenderNormal_VelocityID);
		glBindTexture(GL_TEXTURE_2D, m_RenderNormal_VelocityID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (GLuint)0, GL_TEXTURE_2D, m_RenderNormal_VelocityID, 0);

		// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
		unsigned int attachments[] = { GL_COLOR_ATTACHMENT0 + (GLuint)0 };
		glDrawBuffers(sizeof(attachments) / sizeof(attachments[0]), attachments);

		// Check for completeness/correctness
		int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != int(GL_FRAMEBUFFER_COMPLETE))
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"FBO Error : " + wStr(status));
		}
		// Unbind the fbo until it's ready to be used
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	OpenGLThinGBuffer::~OpenGLThinGBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_DepthID);
		glDeleteTextures(1, &m_RenderNormal_VelocityID);
	}
	void OpenGLThinGBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		GLCHECKERROR;
	}
	void OpenGLThinGBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	void OpenGLThinGBuffer::BindTextures(const std::vector<GBUFFER_TEXTURE_TYPE>& texToBind, uint32_t offset) const
	{
		for (auto& tex : texToBind)
		{
			glBindTextureUnit((GLuint)tex + offset, GetTexutureID(tex));
			GLCHECKERROR;
		}
	}
	uint32_t OpenGLThinGBuffer::GetTexutureID(GBUFFER_TEXTURE_TYPE tex) const
	{
		switch (tex)
		{
		case longmarch::GBuffer::GBUFFER_TEXTURE_TYPE::DEPTH:
			return m_DepthID;
			break;
		case longmarch::GBuffer::GBUFFER_TEXTURE_TYPE::NORMAL_VELOCITY:
			return m_RenderNormal_VelocityID;
			break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Unknow ThinGBuffer texture type!");
		}
	}
	/**************************************************************
	*	Compute Buffer
	**************************************************************/
	OpenGLComputeBuffer::OpenGLComputeBuffer(uint32_t width, uint32_t height, ComputeBuffer::BUFFER_FORMAT type)
	{
		m_width = width;
		m_height = height;
		m_buffer_format = type;

		uint32_t format;
		switch (type)
		{
		case longmarch::ComputeBuffer::BUFFER_FORMAT::Float16:
			format = GL_RGBA16F;
			break;
		case longmarch::ComputeBuffer::BUFFER_FORMAT::Float32:
			format = GL_RGBA32F;
			break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Unknown ComputeBuffer::BUFFER_FORMAT type!");
		}

		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		glGenTextures(1, &m_RenderTargetID);
		glBindTexture(GL_TEXTURE_2D, m_RenderTargetID);
		glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_RenderTargetID, 0);

		// Check for completeness/correctness
		int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != int(GL_FRAMEBUFFER_COMPLETE))
		{
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"FBO Error : " + wStr(status));
		}

		// Unbind the fbo until it's ready to be used
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	OpenGLComputeBuffer::~OpenGLComputeBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_RenderTargetID);
	}
	void OpenGLComputeBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		GLCHECKERROR;
	}
	void OpenGLComputeBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}
	void OpenGLComputeBuffer::BindTexture(uint32_t slot, ComputeBuffer::TEXTURE_BIND_MODE mode) const
	{
		uint32_t format;
		switch (m_buffer_format)
		{
		case longmarch::ComputeBuffer::BUFFER_FORMAT::Float16:
			format = GL_RGBA16F;
			break;
		case longmarch::ComputeBuffer::BUFFER_FORMAT::Float32:
			format = GL_RGBA32F;
			break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Unknown ComputeBuffer::BUFFER_FORMAT type!");
		}
		uint32_t texture_mode;
		switch (mode)
		{
		case longmarch::ComputeBuffer::TEXTURE_BIND_MODE::READ_ONLY:
			texture_mode = GL_READ_ONLY;
			break;
		case longmarch::ComputeBuffer::TEXTURE_BIND_MODE::WRITE_ONLY:
			texture_mode = GL_WRITE_ONLY;
			break;
		case longmarch::ComputeBuffer::TEXTURE_BIND_MODE::READ_WRITE:
			texture_mode = GL_READ_WRITE;
			break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Unknown ComputeBuffer::TEXTURE_BIND_MODE type!");
		}
		glBindImageTexture(slot, m_RenderTargetID, 0, GL_FALSE, 0, texture_mode, format);
	}
}