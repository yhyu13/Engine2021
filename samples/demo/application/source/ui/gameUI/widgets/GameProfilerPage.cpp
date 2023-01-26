#include "application-precompiled-header.h"
#include "GameProfilerPage.h"

void longmarch::GameProfilerPage::Render()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, GetStyle());
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
			char entry[128];
			strcpy(entry, "%.3f %s\t");
			strcat(entry, result.first.c_str());
			ImGui::TextColored(ImGuiUtil::ColWhite, entry, result.second.m_time, result.second.m_timeUnit);
		}
	}
	ImGui::PopStyleVar(1);
	ImGui::PopStyleColor(2);
	ImGui::End();
}