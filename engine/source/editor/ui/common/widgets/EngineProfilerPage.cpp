#include "engine-precompiled-header.h"
#include "EngineProfilerPage.h"
#include "../BaseEngineWidgetManager.h"

void longmarch::EngineProfilerPage::Render()
{
	WIDGET_TOGGLE(KEY_F11);
	WIDGET_EARLY_QUIT();

	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, GetStyle());
	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Engine Profiling", &m_IsVisible, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// Early out if the window is collapsed, as an optimization.
		manager->PopWidgetStyle();
		ImGui::PopStyleVar(1);
		ImGui::PopStyleColor(2);
		ImGui::End();
		return;
	}
	{
		for (const auto& result : Instrumentor::GetEngineInstance()->GetResults()) {
			char entry[100];
			strcpy(entry, "%.3f %s\t");
			strcat(entry, result.first);
			ImGui::TextColored(ImGuiUtil::ColWhite, entry, result.second.m_time, result.second.m_timeUnit);
		}
	}
	
	manager->CaptureMouseAndKeyboardOnHover(true);
	manager->PopWidgetStyle();
	ImGui::PopStyleVar(1);
	ImGui::PopStyleColor(2);
	ImGui::End();
}