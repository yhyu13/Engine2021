#include "engine-precompiled-header.h"
#include "3DEngineWidgetManager.h"

#include "widgets/3DEngineMainMenu.h"
#include "../common/widgets/ComponentInspectorDock.h"
#include "../common/widgets/SceneHierarchyDock.h"
#include "../common/widgets/EngineProfilerPage.h"
#include "../common/widgets/EnginePerformanceMonitor.h"
#include "../common/widgets/EngineConsoleDock.h"
#include "../common/widgets/SceneDock.h"
#include "../common/widgets/EngineEditorHUD.h"

longmarch::_3DEngineWidgetManager::_3DEngineWidgetManager()
{
	{
		auto widget = MemoryManager::Make_shared<EngineEditorHUD>();
		RegisterWidget("0_HUD", widget); // mark with "0_" to place it at the front of std::map when rendering
	}
	{
		auto widget = MemoryManager::Make_shared<_3DEngineMainMenu>();
		RegisterWidget("MainMenu", widget);
	}
	{
		auto widget = MemoryManager::Make_shared<EngineProfilerPage>();
		RegisterWidget("ProfilerPage", widget);
	}
	{
		auto widget = MemoryManager::Make_shared<EnginePerformanceMonitor>();
		RegisterWidget("EnginePerformanceMonitor", widget);
	}
	{
		auto widget = MemoryManager::Make_shared<ComponentInspectorDock>();
		RegisterWidget("ComponentInspectorDock", widget);
	}
	{
		auto widget = MemoryManager::Make_shared<SceneHierarchyDock>();
		RegisterWidget("SceneHierarchyDock", widget);
	}
	{
		auto widget = MemoryManager::Make_shared<EngineConsoleDock>();
		RegisterWidget("EngineConsoleDock", widget);
	} 
	{
		auto widget = MemoryManager::Make_shared<SceneDock>();
		RegisterWidget("~_SceneDock", widget); // mark with "~" to place it the last place
	}
}