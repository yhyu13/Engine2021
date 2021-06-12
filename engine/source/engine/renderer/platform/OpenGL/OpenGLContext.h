#pragma once

#include "engine/renderer/GraphicsContext.h"
#include "OpenGLUtil.h"
#include <GLFW/glfw3.h>

namespace longmarch 
{
	class OpenGLContext final : public GraphicsContext
	{
	public:
		NONCOPYABLE(OpenGLContext);
		explicit OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;
		virtual void RebuildSwapChain(int width, int height) override 
		{ 
			// OpenGL does not need to maully rebuild swap chain
		};
		virtual void* GetNativeWindow() { return m_WindowHandle; };

	private:
		GLFWwindow* m_WindowHandle{ nullptr };
	};
}


