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
		virtual void ResizeSwapChain(int width, int height) override {};
		virtual void* GetNativeWindow() { return m_WindowHandle; };

	private:
		GLFWwindow* m_WindowHandle{ nullptr };
	};
}


