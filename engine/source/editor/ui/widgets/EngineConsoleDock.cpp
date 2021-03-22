#include "engine-precompiled-header.h"
#include "EngineConsoleDock.h"
#include "editor/ui/BaseEngineWidgetManager.h"
#include "engine/core/logging/TerminalLogger.h"

void longmarch::EngineConsoleDock::Render()
{
	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();
	ImVec2 windowsize = ImVec2(GetWindowSize_X(), GetWindowSize_Y());
	ImVec2 mainMenuWindowSize = PosScaleBySize(m_Size, windowsize);
	ImGui::SetNextWindowSize(mainMenuWindowSize, ImGuiCond_Once);

	// Simply invoke terminal logger's render function, and we are done
	TerminalLogger::GetInstance()->show();

	manager->CaptureMouseAndKeyboardOnHover();
	manager->PopWidgetStyle();
}