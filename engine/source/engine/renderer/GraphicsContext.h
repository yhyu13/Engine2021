#pragma once
#include "RenderCommand.h"

namespace longmarch
{
	class GraphicsContext
	{
	public:
		NONCOPYABLE(GraphicsContext);
		GraphicsContext() = default;
		virtual ~GraphicsContext() = default;
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
		virtual void ResizeSwapChain(int width, int height) = 0;
		virtual void* GetNativeWindow() = 0;
	};
}