/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: GAM550
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 12/02/2020
- End Header ----------------------------*/

#include "application-precompiled-header.h"
#include "Prototype2MainMenu.h"
#include "ui/gameUI/BaseGameWidgetManager.h"
#include "ecs/component-systems/controller/Player/Prototype2PlayerArsenalComSys.h"

void longmarch::Prototype2MainMenu::Render()
{
	auto manager = ServiceLocator::GetSingleton<BaseGameWidgetManager>(APP_WIG_MAN_NAME);
	manager->PushWidgetStyle();
	ImGui::PushStyleColor(ImGuiCol_WindowBg, GetStyle());
	ImVec2 windowsize = ImVec2(GetWindowSize_X(), GetWindowSize_Y());
	ImVec2 mainMenuWindowSize = PosScaleBySize(m_Size, windowsize);
	ImGui::SetNextWindowSize(mainMenuWindowSize);

	if (!ImGui::Begin("Prototype2 Main Menu", &m_IsVisible))
	{
		// Early out if the window is collapsed, as an optimization.
		manager->PopWidgetStyle();
		ImGui::PopStyleColor(1);
		ImGui::End();
		return;
	}

	auto prototype2PlayerArsenalComSys = static_cast<Prototype2PlayerArsenalComSys*>(GameWorld::GetCurrent()->GetComponentSystem("Prototype2PlayerArsenalComSys"));

	{
		auto MaxProjectileFireRate = prototype2PlayerArsenalComSys->m_MaxProjectileFireRate;
		if (ImGui::SliderInt("Max Projectile Fire Rate", &MaxProjectileFireRate, 1, 5))
		{
			prototype2PlayerArsenalComSys->m_MaxProjectileFireRate = MaxProjectileFireRate;
		}
	}
	{
		auto MaxNumProjectile = prototype2PlayerArsenalComSys->m_MaxNumProjectile;
		if (ImGui::SliderInt("Max Num Projectile", &MaxNumProjectile, 1, 4))
		{
			prototype2PlayerArsenalComSys->m_MaxNumProjectile = MaxNumProjectile;
		}
	}
	{
		auto MaxNumDrone = prototype2PlayerArsenalComSys->m_MaxNumDrone;
		if (ImGui::SliderInt("Max Num Drone", &MaxNumDrone, 1, 4))
		{
			prototype2PlayerArsenalComSys->m_MaxNumDrone = MaxNumDrone;
		}
	}

	manager->CaptureMouseAndKeyboardOnMenu();
	manager->PopWidgetStyle();
	ImGui::PopStyleColor(1);
	ImGui::End();
}
