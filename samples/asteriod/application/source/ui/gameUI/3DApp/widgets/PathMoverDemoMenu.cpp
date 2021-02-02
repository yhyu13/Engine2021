/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: GAM550
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 10/25/2020
- End Header ----------------------------*/

//***************************************************************************************
//! \file	   PathMoverDemoMenu.h
//! \author    Hang Yu (hang.yu@digipen.edu | 60001119)
//! \date      07/28/2020
//! \copyright Copyright (C) 2020 DigiPen Institute of Technology.
//***************************************************************************************

#include "application-precompiled-header.h"
#include "PathMoverDemoMenu.h"
#include "ui/gameUI/BaseGameWidgetManager.h"
#include "events/CustomEvents.h"
#include "engine/ecs/header/header.h"

#include <imgui/addons/implot/implot.h>

void longmarch::PathMoverDemoMenu::Render()
{
	auto manager = ServiceLocator::GetSingleton<BaseGameWidgetManager>(APP_WIG_MAN_NAME);
	manager->PushWidgetStyle();
	ImGui::PushStyleColor(ImGuiCol_WindowBg, GetStyle());
	ImVec2 windowsize = ImVec2(GetWindowSize_X(), GetWindowSize_Y());
	ImVec2 mainMenuWindowSize = PosScaleBySize(m_Size, windowsize);
	ImGui::SetNextWindowSize(mainMenuWindowSize);
	if (!ImGui::Begin("GAM550 Main Menu", &m_IsVisible))
	{
		// Early out if the window is collapsed, as an optimization.
		manager->PopWidgetStyle();
		ImGui::PopStyleColor(1);
		ImGui::End();
		return;
	}

	RenderPathMotionMenu();

	manager->CaptureMouseAndKeyboardOnMenu();
	manager->PopWidgetStyle();
	ImGui::PopStyleColor(1);
	ImGui::End();
}

void longmarch::PathMoverDemoMenu::RenderPathMotionMenu()
{
	constexpr int yoffset_item = 2;
	constexpr int width_item = 100;
	if (ImGui::TreeNode("Path Motion"))
	{
		if (ImGui::BeginTabBar("Path Motion Settings"))
		{
			auto world = GameWorld::GetCurrent();
			auto entityType = (EntityType)GameEntityType::NPC;
			auto npcs = world->GetAllEntityWithType(entityType);
			EntityDecorator pathAgent;
			for (auto& npc : npcs)
			{
				if (world->GetComponent<IDNameCom>(npc)->GetName() == "YBot") //! Simply pick one simple animated character
				{
					pathAgent = EntityDecorator{ npc , world };
					break;
				}
			}

			static float movement_speed = 2.5f;
			static bool pause = false;

			static float speed_ramp_up_time = 1.0f;
			static float speed_ramp_down_time = 1.0f;

			static float angle_degree_tolerance = 175.0f;
			static float delta_distance_tolerance = 0.1f;

			{
				float speed = .05f;
				float range = 2.f;
				ImGui::PushItemWidth(width_item);
				if (ImGui::DragFloat("Movement Speed", &movement_speed, speed, movement_speed - range, movement_speed + range))
				{
					movement_speed = MAX(0.001f, movement_speed);
				}
			}

			{
				float speed = .01f;
				float range = 2.f;
				ImGui::PushItemWidth(width_item);
				if (ImGui::DragFloat("Speed Ease In", &speed_ramp_up_time, speed, speed_ramp_up_time - range, speed_ramp_up_time + range))
				{
					speed_ramp_up_time = MAX(0.f, speed_ramp_up_time);
				}
			}

			{
				float speed = .01f;
				float range = 2.f;
				ImGui::PushItemWidth(width_item);
				if (ImGui::DragFloat("Speed Ease Out", &speed_ramp_down_time, speed, speed_ramp_down_time - range, speed_ramp_down_time + range))
				{
					speed_ramp_down_time = MAX(0.f, speed_ramp_down_time);
				}
			}

			{
				if (ImGui::Button("Generate Random Path"))
				{
					// Generate path
					{
						auto queue = EventQueue<CS560EventType>::GetInstance();
						auto e = MemoryManager::Make_shared<CS560GenRndPathEvent>(pathAgent);
						queue->Publish(e);
					}
					// Update path control
					{
						bool _restart = true;
						bool _pause = true;
						auto queue = EventQueue<CS560EventType>::GetInstance();
						auto e = MemoryManager::Make_shared<CS560PatherControlEvent>(pathAgent, _restart, _pause);
						queue->Publish(e);
					}
				}
				ImGuiUtil::InlineHelpMarker("Press this button to generate a new random path.");
			}
			ImGui::Separator();
			// Smoothing
			{
				{
					float speed = .05f;
					float range = 2.f;
					if (ImGui::DragFloat("Smooth angle tolerance", &angle_degree_tolerance, speed, angle_degree_tolerance - range, angle_degree_tolerance + range))
					{
						angle_degree_tolerance = Geommath::Clamp(angle_degree_tolerance, 0.0001f, 175.0f);
					}
				}

				{
					float speed = .01f;
					float range = 2.f;
					if (ImGui::DragFloat("Smooth distance tolerance", &delta_distance_tolerance, speed, delta_distance_tolerance - range, delta_distance_tolerance + range))
					{
						delta_distance_tolerance = MAX(0.0001f, delta_distance_tolerance);
					}
				}

				{
					if (ImGui::Button("Update Smoothing method"))
					{
						// Update smoothing setting
						{
							auto queue = EventQueue<CS560EventType>::GetInstance();
							auto e = MemoryManager::Make_shared<CS560PatherSmoothSettingEvent>(pathAgent, angle_degree_tolerance * DEG2RAD, delta_distance_tolerance);
							queue->Publish(e);
						}
						// Update path misc
						{
							auto queue = EventQueue<CS560EventType>::GetInstance();
							auto e = MemoryManager::Make_shared<CS560PatherMiscEvent>(pathAgent, movement_speed, speed_ramp_up_time, speed_ramp_down_time);
							queue->Publish(e);
						}
						// Update path control
						{
							bool _restart = true;
							bool _pause = pause;
							auto queue = EventQueue<CS560EventType>::GetInstance();
							auto e = MemoryManager::Make_shared<CS560PatherControlEvent>(pathAgent, _restart, _pause);
							queue->Publish(e);
						}
					}
					ImGuiUtil::InlineHelpMarker("Press this button to update the smoothing settings.");
				}
			}
			ImGui::Separator();
			// Control
			{
				{
					if (ImGui::Button("Start/Restart Walking"))
					{
						// Update path misc
						{
							auto queue = EventQueue<CS560EventType>::GetInstance();
							auto e = MemoryManager::Make_shared<CS560PatherMiscEvent>(pathAgent, movement_speed, speed_ramp_up_time, speed_ramp_down_time);
							queue->Publish(e);
						}
						// Update path control
						{
							bool _restart = true;
							bool _pause = pause;
							auto queue = EventQueue<CS560EventType>::GetInstance();
							auto e = MemoryManager::Make_shared<CS560PatherControlEvent>(pathAgent, _restart, _pause);
							queue->Publish(e);
						}
					}
					ImGuiUtil::InlineHelpMarker("Press this button to begin moving on a path.");
				}

				if (ImGui::Checkbox("Pause", &pause))
				{
					// Update path control
					{
						bool _restart = false;
						bool _pause = pause;
						auto queue = EventQueue<CS560EventType>::GetInstance();
						auto e = MemoryManager::Make_shared<CS560PatherControlEvent>(pathAgent, _restart, _pause);
						queue->Publish(e);
					}
				}
			}

			ImGui::EndTabBar();
		}
		ImGui::Separator();
		ImGui::TreePop();
	}
}