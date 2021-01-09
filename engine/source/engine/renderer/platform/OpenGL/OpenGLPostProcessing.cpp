#include "engine-precompiled-header.h"
#include "OpenGLPostProcessing.h"
#include "engine/core/file-system/FileSystem.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "engine/core/file-system/FileSystem.h"

namespace AAAAgames {
	OpenGLPostProcessing::OpenGLPostProcessing()
	{
	}

	OpenGLPostProcessing::OpenGLPostProcessing(uint32_t screenWidth, uint32_t screenHeight)
		: m_ScreenWidth(screenWidth), m_ScreenHeight(screenHeight)
	{
	}

	OpenGLPostProcessing::~OpenGLPostProcessing()
	{
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_VBO);
		glDeleteRenderbuffers(1, &m_DepthID);
		glDeleteTextures(1, &m_RenderTargetID);
		glDeleteFramebuffers(1, &m_FrameBufferID);
	}

	void OpenGLPostProcessing::Init()
	{
		m_Shader = Shader::Create("$shader:PostProcessing_ContractColor.vert", "$shader:PostProcessing_ContractColor.frag");
		float asp = (float)m_ScreenWidth / (float)m_ScreenHeight;

		//Screen Quad VAO
		float quadVertices[] = {
			-asp,  asp,  0.0f, 1.0f,
			-asp, -asp,  0.0f, 0.0f,
			 asp, -asp,  1.0f, 0.0f,

			-asp,  asp,  0.0f, 1.0f,
			 asp, -asp,  1.0f, 0.0f,
			 asp,  asp,  1.0f, 1.0f
		};
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glBindVertexArray(0);

		m_Shader->Bind();
		m_Shader->SetInt("u_Texture", 0);

		//Framebuffer configuration
		glGenFramebuffers(1, &m_FrameBufferID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);

		//Create a texture of render target
		glGenTextures(1, &m_RenderTargetID);
		glBindTexture(GL_TEXTURE_2D, m_RenderTargetID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_ScreenWidth, m_ScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_RenderTargetID, 0);

		//Create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		glGenRenderbuffers(1, &m_DepthID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_DepthID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_ScreenWidth, m_ScreenHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthID);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLPostProcessing::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);
		glEnable(GL_DEPTH_TEST);
	}

	void OpenGLPostProcessing::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void OpenGLPostProcessing::Render(float ts)
	{
		m_Shader->Bind();
		m_Shader->SetInt("u_Texture", 0);
		glBindTexture(GL_TEXTURE_2D, m_RenderTargetID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_ScreenWidth, m_ScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glBindVertexArray(m_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
}