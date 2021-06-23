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

		virtual void Init() override;
		virtual void SwapBuffers() override;
		virtual void RebuildSwapChain(int width, int height) override 
		{ 
			// OpenGL does not need to maully rebuild swap chain
		};
		virtual void* GetNativeWindow() { return m_WindowHandle; };

	private:
		explicit OpenGLContext(GLFWwindow* windowHandle);

	public:
		[[nodiscard]] static OpenGLContext* Create(GLFWwindow* windowHandle)
		{
			if (LongMarch_contains(s_windowContextMap, windowHandle)) [[unlikely]]
			{
				ENGINE_EXCEPT(L"Unhandled duplicate OpenGL context!");
				return nullptr;
			}
			else [[likely]]
			{
				auto ret = new OpenGLContext(windowHandle);
				s_windowContextMap[windowHandle] = ret;
				return ret;
			}
		}
		static void Destory(GLFWwindow* windowHandle)
		{
			if (auto it = s_windowContextMap.find(windowHandle); it != s_windowContextMap.end())
			{
				ASSERT(dynamic_cast<OpenGLContext*>(it->second), "Failed to cast to OpenGLContext!");
				delete it->second;
				s_windowContextMap.erase(it);
			}
			else
			{
				ENGINE_EXCEPT(L"Unhandled erase of empty OpenGL context!");
			}
		}

	private:
		GLFWwindow* m_WindowHandle{ nullptr };
	};
}


