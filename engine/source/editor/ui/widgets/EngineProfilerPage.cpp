#include "engine-precompiled-header.h"
#include "EngineProfilerPage.h"

void AAAAgames::EngineProfilerPage::Render()
{
	auto instrumentor = Instrumentor::GetEngineInstance();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, GetStyle());
	//ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);			// deprecated with new ImGui updates
	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Engine Profiling", &m_IsVisible, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::PopStyleVar(1);
		ImGui::PopStyleColor(2);
		ImGui::End();
		return;
	}
	{
		for (const auto& result : instrumentor->GetResults()) {
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