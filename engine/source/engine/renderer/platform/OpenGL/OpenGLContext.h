#pragma once

#include "../../GraphicsContext.h"
#include "OpenGLUtil.h"

struct GLFWwindow;

namespace longmarch {

	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_WindowHandle;
	};
}


