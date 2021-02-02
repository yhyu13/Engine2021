/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: CS562
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 07/28/2020
- End Header ----------------------------*/

#include "application-precompiled-header.h"
#include "GameProfilerPage.h"

void longmarch::GameProfilerPage::Render()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, GetStyle());
	//ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);			// deprecated with new ImGui updates
	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Application Profiling", &m_IsVisible, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::PopStyleVar(1);
		ImGui::PopStyleColor(2);
		ImGui::End();
		return;
	}
	{
		for (const auto& result : Instrumentor::GetApplicationInstance()->GetResults()) {
			char entry[100];
			strcpy(entry, "%10.3f %s\t");
			strcat(entry, result.first);
			ImGui::TextColored(ImGuiUtil::ColWhite, entry, result.second.m_time, result.second.m_timeUnit);
		}
	}
	ImGui::PopStyleVar(1);
	ImGui::PopStyleColor(2);
	ImGui::End();
}