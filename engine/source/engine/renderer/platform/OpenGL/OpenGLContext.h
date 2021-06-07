#pragma once

#include "../../GraphicsContext.h"
#include "OpenGLUtil.h"
#include <GLFW/glfw3.h>

namespace longmarch 
{
	class OpenGLContext final : public GraphicsContext
	{
	public:
		explicit OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_WindowHandle;
	};
}


