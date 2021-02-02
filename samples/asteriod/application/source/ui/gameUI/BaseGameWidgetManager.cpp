/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: GAM541
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 07/28/2020
- End Header ----------------------------*/

//***************************************************************************************
//! \file	   BaseGameWidgetManager.h
//! \author    Hang Yu (hang.yu@digipen.edu | 60001119)
//! \date      07/28/2020
//! \copyright Copyright (C) 2020 DigiPen Institute of Technology.
//***************************************************************************************

#include "application-precompiled-header.h"
#include "BaseGameWidgetManager.h"

namespace longmarch
{
	void BaseGameWidgetManager::RenderUI()
	{
		// Render widgets
		BaseWidgetManager::RenderUI();
		CaptureMouseAndKeyboardOnMenu();
	}

	void BaseGameWidgetManager::CaptureMouseAndKeyboardOnMenu()
	{
		ImGuiIO& io = ImGui::GetIO();
		//bool isWindowFocused = ImGui::IsRootWindowOrAnyChildHovered() || ImGui::IsAnyItemHovered(); // deprecated with ImGui update.
		bool isWindowFocused = ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered();
		// Set ImGui capture inputs on window focused
		if (isWindowFocused)
		{
			io.WantCaptureMouse = isWindowFocused;
			io.WantCaptureKeyboard = isWindowFocused;
		}
	}

	void BaseGameWidgetManager::LoadWidget(const fs::path& filepath)
	{
		const auto& json = FileSystem::GetCachedJsonCPP(filepath);
		auto styles = json["bg-Color"];
		{
			for (auto& [name, widget] : m_WidgetLUT)
			{
				auto& style = styles[name];
				if (!style.isNull())
				{
					widget->SetStyle(style[0].asInt(), style[1].asInt(), style[2].asInt(), style[3].asInt());
				}
				else
				{
					widget->SetStyle(51, 51, 51, 127); // Default background color
				}
			}
		}
	}
}