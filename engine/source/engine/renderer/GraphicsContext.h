#pragma once
#include "engine/core/EngineCore.h"
#include "engine/core/utility/TypeHelper.h"

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
		//! RebuildSwapChain with both width and hieght equal to -1 indicates to query glfw window extent
		virtual void RebuildSwapChain(int width = -1, int height = -1) = 0;
		virtual void* GetNativeWindow() = 0;

	protected:
		inline static LongMarch_UnorderedMap_flat<void*, GraphicsContext*> s_windowContextMap;
	};
}