#pragma once
#include "ImGuiUtil.h"
#include "engine/core/EngineCore.h"

namespace longmarch
{
	class ENGINE_API ImGuiDriver
	{
	public:
		static void Init();
		static void ShutDown();
		static void BeginFrame();
		static void EndFrame();		
	};
}
