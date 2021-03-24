#include "engine-precompiled-header.h"
#include "SceneDock.h"
#include "editor/ui/BaseEngineWidgetManager.h"
#include "engine/renderer/Renderer3D.h"

longmarch::SceneDock::SceneDock()
{
	m_IsVisible = true;
	m_Size = ScaleSize({ 800, 600 });
}

void longmarch::SceneDock::Render()
{
	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();
	ImVec2 windowsize = ImVec2(GetWindowSize_X(), GetWindowSize_Y());
	ImVec2 mainMenuWindowSize = PosScaleBySize(m_Size, windowsize);
	ImGui::SetNextWindowSize(mainMenuWindowSize, ImGuiCond_Once);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	if (!ImGui::Begin("Scene", &m_IsVisible, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav))
	{
		// Early out if the window is collapsed, as an optimization.
		manager->PopWidgetStyle(); 
		ImGui::PopStyleVar(1);
		ImGui::End();
		return;
	}

	auto pos = ImGui::GetCurrentWindowRead()->Pos;
	//auto rect = ImGui::GetCurrentWindowRead()->Rect();
	auto size = ImGui::GetCurrentWindowRead()->Size;
	auto contentSize = ImGui::GetCurrentWindowRead()->ContentSize;
	PRINT(Str("Scene Dock pos : %.2f, %.2f | size : %.2f, %.2f | content size : %.2f, %.2f", pos.x, pos.y, size.x, size.y, contentSize.x, contentSize.y));

	auto buffer = Renderer3D::s_Data.gpuBuffer.CurrentFinalFrameBuffer;
	ImGui::Image(reinterpret_cast<ImTextureID>(buffer->GetRenderTargetID()), ImVec2(buffer->GetBufferSize().x, buffer->GetBufferSize().y), ImVec2(0, 1), ImVec2(1, 0));

	{
		// Mouse over scene dock should not be captured by ImGui
		bool isWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive();
		if (isWindowHovered)
		{
			ImGuiUtil::IgnoreMouseCaptured = ImGuiUtil::IgnoreKeyBoardCaptured = true;
		}
	}
	manager->PopWidgetStyle();
	ImGui::PopStyleVar(1);
	ImGui::End();
}
