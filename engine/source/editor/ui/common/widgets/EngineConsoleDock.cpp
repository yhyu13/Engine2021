#include "engine-precompiled-header.h"
#include "EngineConsoleDock.h"
#include "../BaseEngineWidgetManager.h"
#include "engine/core/logging/TerminalLogger.h"

longmarch::EngineConsoleDock::EngineConsoleDock()
{
	m_IsVisible = true;
	m_Size = ScaleSize({ 500, 400 });
}

void longmarch::EngineConsoleDock::Render()
{
	WIDGET_TOGGLE(KEY_F11);
	WIDGET_EARLY_QUIT();

	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();
	ImVec2 windowsize = ImVec2(GetWindowSize_X(), GetWindowSize_Y());
	ImVec2 mainMenuWindowSize = PosScaleBySize(m_Size, windowsize);
	ImGui::SetNextWindowSize(mainMenuWindowSize, ImGuiCond_Once);

	// Simply invoke terminal logger's render function, and we are done
	TerminalLogger::GetInstance()->show();

	manager->CaptureMouseAndKeyboardOnHover(true);
	manager->PopWidgetStyle();
}