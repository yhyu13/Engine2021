#include "engine-precompiled-header.h"
#include "OpenGLContext.h"
#include "OpenGLRendererAPI.h"
#include "engine/renderer/RenderCommand.h"

#include <glad/glad.h>
#include <gl/GL.h>

namespace longmarch 
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: 
		m_WindowHandle(windowHandle)
	{
		ASSERT(windowHandle != nullptr, "Window handle is NULL!");
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		ASSERT(status >= 0, "Failed to initialize Glad");

		ENGINE_INFO(" OpenGL Info:");
		ENGINE_INFO(" Vender: {0}", glGetString(GL_VENDOR));
		ENGINE_INFO(" Renderer: {0}", glGetString(GL_RENDERER));
		ENGINE_INFO(" Version: {0}", glGetString(GL_VERSION));
		
		RenderCommand::SetAPI(OpenGLRendererAPI::GetInstance());
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}