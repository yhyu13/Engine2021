#include "application-precompiled-header.h"
#include "3DGameWidgetManager.h"
#include "ui/gameUI/GameHUD.h"
#include "ui/gameUI/widgets/GameMainMenu.h"
#include "ui/gameUI/widgets/GameProfilerPage.h"
#include "ui/gameUI/3DApp/widgets/PathMoverDemoMenu.h"

longmarch::_3DGameWidgetManager::_3DGameWidgetManager()
{
	{
		auto widget = MemoryManager::Make_shared<GameHUD>();
		RegisterWidget("0_HUD", widget);
	}
	{
		auto widget = MemoryManager::Make_shared<GameMainMenu>();
		RegisterWidget("MainMenu", widget);
	}
	{
		auto widget = MemoryManager::Make_shared<GameProfilerPage>();
		RegisterWidget("ProfilerPage", widget);
	}
	{
		// Prototypes do no need path mover 
		auto widget = MemoryManager::Make_shared<PathMoverDemoMenu>();
		RegisterWidget("PathMoverDemoMenu", widget);
	}
	LoadWidget(FileSystem::ResolveProtocol("$asset:archetype/widget.json"));
}