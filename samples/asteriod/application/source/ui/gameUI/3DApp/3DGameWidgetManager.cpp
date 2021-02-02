/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: CS562
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 09/07/2020
- End Header ----------------------------*/

//***************************************************************************************
//! \file	   3DGameWidgetManager.cpp
//! \author    Hang Yu (hang.yu@digipen.edu | 60001119)
//! \date      07/28/2020
//! \copyright Copyright (C) 2020 DigiPen Institute of Technology.
//***************************************************************************************

#include "application-precompiled-header.h"
#include "3DGameWidgetManager.h"
#include "ui/gameUI/GameHUD.h"
#include "ui/gameUI/widgets/GameMainMenu.h"
#include "ui/gameUI/widgets/GameProfilerPage.h"
#include "ui/gameUI/3DApp/widgets/PathMoverDemoMenu.h"
#include "ui/gameUI/3DApp/widgets/Prototype2MainMenu.h"

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
		auto widget = MemoryManager::Make_shared<Prototype2MainMenu>();
		RegisterWidget("Prototype2MainMenu", widget);
	}
	{
		// Prototypes do no need path mover 
		/*auto widget = MemoryManager::Make_shared<PathMoverDemoMenu>();
		RegisterWidget("PathMoverDemoMenu", widget);*/
	}
	LoadWidget(("$asset:archetype/widget.json"));
}