#pragma once
#include "ImGuiUtil.h"
#include "engine/core/EngineCore.h"

namespace longmarch
{
	struct ENGINE_API ImGuiDriver
	{
		static void Init();
		static void ShutDown();
		static void BeginFrame();
		static void EndFrame();
	};
}
