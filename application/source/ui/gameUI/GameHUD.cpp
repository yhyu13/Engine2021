#include "application-precompiled-header.h"
#include "GameHUD.h"

void AAAAgames::GameHUD::Render()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowBgAlpha(0.0f);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
	window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
	window_flags |= ImGuiWindowFlags_NoDecoration;
	window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
	window_flags |= ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleColor(ImGuiCol_WindowBg, GetStyle());
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	if (!ImGui::Begin("GameHUD", &m_IsVisible, window_flags))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor(1);
		ImGui::End();
		return;
	}
	ImGui::PopStyleVar(3);
	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("GameHUDDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	}

	ShowEngineFPS();
	ImGui::PopStyleColor(1);
	ImGui::End();
}

void AAAAgames::GameHUD::ShowEngineFPS()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	float frameTime = FramerateController::GetInstance()->GetFrameTime();
	ImVec2 waveWindowSize = ImVec2(90, 50);
	ImVec2 waveWindowPos = ImVec2(viewport->Pos.x + viewport->Size.x / 2 - waveWindowSize.x / 2, viewport->Pos.y + 50);
	ImGui::SetNextWindowPos(waveWindowPos);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0, 0.0, 0.0, 0.0));
	ImGui::PushStyleColor(ImGuiCol_Text, ImGuiUtil::ColGreen);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	ImGui::BeginChild("FPS", waveWindowSize, false, ImGuiUtil::menuFlag);
	ImGui::SetCursorPosX(0.0f);
	ImGui::SetCursorPosY(ImGui::GetFontSize() / 4);
	ImGui::Text("%.2f FPS \n%.2f ms", 1.0f / frameTime, frameTime * 1e3);
	ImGui::EndChild();
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(2);
}
