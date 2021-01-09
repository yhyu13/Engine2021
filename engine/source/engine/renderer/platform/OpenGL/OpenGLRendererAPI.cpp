#include "engine-precompiled-header.h"
#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace longmarch {
	void OpenGLRendererAPI::Init()
	{
		// Use [0,1] in NDC for depth values
		glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

		// Enable depth test
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		
		//glEnable(GL_SCISSOR_TEST); // Scissor test wouldn't allow variable resolution rendering

		// Enable stencil test
		glStencilMask(GL_TRUE);
		glEnable(GL_STENCIL_TEST);

		// Enable facing culling
		// Culling back face
		// Front face as counter clock wise
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		// Blending
		glEnable(GL_BLEND);
		

		// Cubemap semaless
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}
	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}
	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::ClearColorOnly()
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void OpenGLRendererAPI::ClearDepthOnly()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	GLuint OpenGLRendererAPI::CreateAndBindFBO()
	{
		GLuint fbo;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		return fbo;
	}

	void OpenGLRendererAPI::DestoryAndUnBindFBO(unsigned int fbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo);
	}

	void OpenGLRendererAPI::DrawTriangleIndexed(const std::shared_ptr<VertexArray>& vertexArray)
	{
		GLenum indexFormat;
		size_t elementSize = vertexArray->GetIndexBuffer()->GetElementSize();
		switch (elementSize)
		{
		case 1:
			indexFormat = GL_UNSIGNED_BYTE;
			break;
		case 2:
			indexFormat = GL_UNSIGNED_SHORT;
			break;
		case 4:
			indexFormat = GL_UNSIGNED_INT;
			break;
		}
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), indexFormat, nullptr);
	}

	void OpenGLRendererAPI::DrawLineIndexed(const std::shared_ptr<VertexArray>& vertexArray)
	{
		GLenum indexFormat;
		size_t elementSize = vertexArray->GetIndexBuffer()->GetElementSize();
		switch (elementSize)
		{
		case 1:
			indexFormat = GL_UNSIGNED_BYTE;
			break;
		case 2:
			indexFormat = GL_UNSIGNED_SHORT;
			break;
		case 4:
			indexFormat = GL_UNSIGNED_INT;
			break;
		}
		glLineWidth(2);
		glDrawElements(GL_LINES, vertexArray->GetIndexBuffer()->GetCount(), indexFormat, nullptr);
		glLineWidth(1);
	}

	void OpenGLRendererAPI::DrawTriangleIndexedInstanced(const std::shared_ptr<VertexArray>& vertexArray, uint32_t count)
	{
		GLenum indexFormat;
		size_t elementSize = vertexArray->GetIndexBuffer()->GetElementSize();
		switch (elementSize)
		{
		case 1:
			indexFormat = GL_UNSIGNED_BYTE;
			break;
		case 2:
			indexFormat = GL_UNSIGNED_SHORT;
			break;
		case 4:
			indexFormat = GL_UNSIGNED_INT;
			break;
		}
		glDrawElementsInstanced(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), indexFormat, nullptr, count);
	}

	void OpenGLRendererAPI::DrawLineIndexedInstanced(const std::shared_ptr<VertexArray>& vertexArray, uint32_t count)
	{
		GLenum indexFormat;
		size_t elementSize = vertexArray->GetIndexBuffer()->GetElementSize();
		switch (elementSize)
		{
		case 1:
			indexFormat = GL_UNSIGNED_BYTE;
			break;
		case 2:
			indexFormat = GL_UNSIGNED_SHORT;
			break;
		case 4:
			indexFormat = GL_UNSIGNED_INT;
			break;
		}
		glLineWidth(2);
		glDrawElementsInstanced(GL_LINES, vertexArray->GetIndexBuffer()->GetCount(), indexFormat, nullptr, count);
		glLineWidth(1);
	}

	void OpenGLRendererAPI::MultiDrawTriangleIndexedIndirect(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<IndexedIndirectCommandBuffer>& commandBuffer)
	{
		GLenum indexFormat;
		size_t elementSize = vertexArray->GetIndexBuffer()->GetElementSize();
		switch (elementSize)
		{
		case 1:
			indexFormat = GL_UNSIGNED_BYTE;
			break;
		case 2:
			indexFormat = GL_UNSIGNED_SHORT;
			break;
		case 4:
			indexFormat = GL_UNSIGNED_INT;
			break;
		}
		glMultiDrawElementsIndirect(GL_TRIANGLES, indexFormat, nullptr, commandBuffer->GetCount(), 0);
	}

	void OpenGLRendererAPI::DispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
	{
		glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
	}

	void OpenGLRendererAPI::PolyModeFill() 
	{ 
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
	}

	void OpenGLRendererAPI::PolyModeLine() 
	{ 
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
	}

	void OpenGLRendererAPI::BindDefaultFrameBuffer() 
	{ 
		glBindFramebuffer(GL_FRAMEBUFFER, 0); 
	}

	void OpenGLRendererAPI::TransferColorBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h)
	{
		/*
			For OpenGL 4.5 glBlitNamedFramebuffer is pipelined, use with caution
			So here we remain using the old method.
		*/
		glBindFramebuffer(GL_READ_FRAMEBUFFER, src);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest);
		glBlitFramebuffer(
			0, 0, src_w, src_h, 0, 0, dest_w, dest_h, GL_COLOR_BUFFER_BIT, GL_LINEAR
		);
		GLCHECKERROR;
	}

	void OpenGLRendererAPI::TransferColorBit(uint32_t src, uint32_t src_tex, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_tex, uint32_t dest_w, uint32_t dest_h)
	{
		// Method 1
		// Bind input FBO + texture to a color attachment
		glBindFramebuffer(GL_READ_FRAMEBUFFER, src);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, src_tex, 0);
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		// Bind destination FBO + texture to another color attachment
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, dest_tex, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);

		// specify source, destination drawing (sub)rectangles.
		glBlitFramebuffer(0, 0, src_w, src_h,
			0, 0, dest_w, dest_h,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);

		// unbind the color attachments
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		
		// Method 2
		//glBindFramebuffer(GL_READ_FRAMEBUFFER, src);
		//glReadBuffer(GL_COLOR_ATTACHMENT0);
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest);
		//GLenum drawbuf = GL_COLOR_ATTACHMENT0;
		//glDrawBuffers(1, &drawbuf);
		//glBlitFramebuffer(0, 0, src_w, src_h, 0, 0, dest_w, dest_h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLCHECKERROR;
	}

	void OpenGLRendererAPI::TransferDepthBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h)
	{
		/*
			For OpenGL 4.5 glBlitNamedFramebuffer is pipelined, use with caution
			So here we remain using the old method.
		*/
		glBindFramebuffer(GL_READ_FRAMEBUFFER, src);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest);
		glBlitFramebuffer(
			0, 0, src_w, src_h, 0, 0, dest_w, dest_h, GL_DEPTH_BUFFER_BIT, GL_NEAREST
		);
		GLCHECKERROR;
	}

	void OpenGLRendererAPI::TransferStencilBit(uint32_t src, uint32_t src_w, uint32_t src_h, uint32_t dest, uint32_t dest_w, uint32_t dest_h)
	{
		/*
			For OpenGL 4.5 glBlitNamedFramebuffer is pipelined, use with caution
			So here we remain using the old method.
		*/
		glBindFramebuffer(GL_READ_FRAMEBUFFER, src);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest);
		glBlitFramebuffer(
			0, 0, src_w, src_h, 0, 0, dest_w, dest_h, GL_STENCIL_BUFFER_BIT, GL_NEAREST
		);
		GLCHECKERROR;
	}
	void OpenGLRendererAPI::Reverse_Z(bool enabled)
	{
		static auto b = !enabled;
		if (b == enabled)
		{
			return;
		}
		b = enabled;

		if (b)
		{
			// Enable Reverse - Z
			glClearDepth(0.0f);
			glDepthFunc(GL_GREATER);
		}
		else
		{
			// Disable Reverse - Z
			glClearDepth(1.0f);
			glDepthFunc(GL_LESS);
		}
	}

	void OpenGLRendererAPI::DepthTest(bool test, bool write)
	{
		static auto b1 = !test;
		static auto b2 = !write;
		if (b1 == test && b2 == write)
		{
			return;
		}
		b1 = test;
		b2 = write;

		if (test)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
		if (write)
		{
			if (!test)
			{
				throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Write to depth buffer requires depth testing to be enabled!");
			}
			glDepthMask(GL_TRUE);
		}
		else
		{
			glDepthMask(GL_FALSE);
		}
	}

	void OpenGLRendererAPI::DepthFunc(longmarch::RendererAPI::CompareEnum e)
	{
		switch (e)
		{
		case longmarch::RendererAPI::CompareEnum::LESS:
			glDepthFunc(GL_LESS);
			break;
		case longmarch::RendererAPI::CompareEnum::LEQUAL:
			glDepthFunc(GL_LEQUAL);
			break;
		case longmarch::RendererAPI::CompareEnum::EQUAL:
			glDepthFunc(GL_EQUAL);
			break;
		case longmarch::RendererAPI::CompareEnum::GEQUAL:
			glDepthFunc(GL_GEQUAL);
			break;
		case longmarch::RendererAPI::CompareEnum::GREATER:
			glDepthFunc(GL_GREATER);
			break;
		default:
			ENGINE_EXCEPT(L"Unkown RendererAPI::CompareEnum!");
			break;
		}
	}

	void OpenGLRendererAPI::StencilTest(bool test, bool write)
	{
		static auto b1 = !test;
		static auto b2 = !write;
		if (b1 == test && b2 == write)
		{
			return;
		}
		b1 = test;
		b2 = write;

		if (test)
		{
			glEnable(GL_STENCIL_TEST);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}
		if (write)
		{
			glStencilMask(GL_TRUE);
		}
		else
		{
			glStencilMask(GL_FALSE);
		}
	}
	void OpenGLRendererAPI::CullFace(bool enabled, bool front)
	{
		static auto b1 = !enabled;
		static auto b2 = !front;
		if (b1 == enabled && b2 == front)
		{
			return;
		}
		b1 = enabled;
		b2 = front;

		if (enabled)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}
		if (front)
		{
			glCullFace(GL_FRONT);
		}
		else
		{
			glCullFace(GL_BACK);
		}
	}
	void OpenGLRendererAPI::Blend(bool enabled)
	{
		static auto b = !enabled;
		if (b == enabled)
		{
			return;
		}
		b = enabled;

		if (enabled)
		{
			glEnable(GL_BLEND);
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}
	void OpenGLRendererAPI::BlendFunc(longmarch::RendererAPI::BlendFuncEnum e)
	{
		switch (e)
		{
		case longmarch::RendererAPI::BlendFuncEnum::ADDITION:
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);
			break;
		case longmarch::RendererAPI::BlendFuncEnum::ALPHA_BLEND_1:
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
			break;
		case longmarch::RendererAPI::BlendFuncEnum::ALPHA_BLEND_2:
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case longmarch::RendererAPI::BlendFuncEnum::ONE_MINUS_SRC_APLHA:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		default:
			ENGINE_EXCEPT(L"Unkown RendererAPI::BlendFuncEnum!");
			break;
		}
	}
	void OpenGLRendererAPI::DepthClamp(bool enabled)
	{
		static auto b = !enabled;
		if (b == enabled)
		{
			return;
		}
		b = enabled;
		
		if (enabled)
		{
			glEnable(GL_DEPTH_CLAMP);
		}
		else
		{
			glDisable(GL_DEPTH_CLAMP);
		}
	}
}

