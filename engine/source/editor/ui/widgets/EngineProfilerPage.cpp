#include "engine-precompiled-header.h"
#include "EngineProfilerPage.h"
#include "../BaseEngineWidgetManager.h"

void longmarch::EngineProfilerPage::Render()
{
	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();
	auto instrumentor = Instrumentor::GetEngineInstance();
	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Engine Profiling", &m_IsVisible, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// Early out if the window is collapsed, as an optimization.
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

	manager->CaptureMouseAndKeyboardOnHover();
	manager->PopWidgetStyle();
	ImGui::End();
}