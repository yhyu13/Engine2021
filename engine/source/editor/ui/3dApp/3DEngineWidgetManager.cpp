#include "engine-precompiled-header.h"
#include "3DEngineWidgetManager.h"

#include "widgets/3DEngineMainMenu.h"
#include "../widgets/ComponentInspectorDock.h"
#include "../widgets/SceneHierarchyDock.h"
#include "../widgets/EngineProfilerPage.h"
#include "../widgets/EngineConsoleDock.h"
#include "../EngineEditorDock.h"

AAAAgames::_3DEngineWidgetManager::_3DEngineWidgetManager()
{
	{
		// mark with "0_" to place it at the front of std::map fpr rendering
		auto widget = MemoryManager::Make_shared<EngineEditorDock>();
		RegisterWidget("0_HUD", widget); 
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
}