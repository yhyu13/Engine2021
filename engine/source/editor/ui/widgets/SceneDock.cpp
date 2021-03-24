#include "engine-precompiled-header.h"
#include "SceneDock.h"
#include "editor/ui/BaseEngineWidgetManager.h"
#include "engine/ecs/header/header.h"
#include "engine/renderer/Renderer3D.h"
#include "editor/ecs/header/header.h"

longmarch::SceneDock::SceneDock()
{
	m_IsVisible = true;
	m_Size = ScaleSize({ 800, 600 });
}

void longmarch::SceneDock::Render()
{
	// Feature disabled, because
	// 1, this feature currently only works properly on fully expanded window or full screen. It seems GLFW's cursor position does not glue well with ImGui's window view port, try to debug break on GenerateRayFromCursorSpace() method in PickingPass3D.cpp
	// 2, Resize scene dock seems to cause frame drop drastically, making editor harder to approach.
	return;

	//auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	//manager->PushWidgetStyle();
	//ImVec2 windowsize = ImVec2(GetWindowSize_X(), GetWindowSize_Y());
	//ImVec2 mainMenuWindowSize = PosScaleBySize(m_Size, windowsize);
	//ImGui::SetNextWindowSize(mainMenuWindowSize, ImGuiCond_Once);

	//// Broderless and no padding, result in a clean capture of the whole scene
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f); 
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));

	//if (!ImGui::Begin("Scene", &m_IsVisible, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav))
	//{
	//	// Early out if the window is collapsed, as an optimization.
	//	manager->PopWidgetStyle(); 
	//	ImGui::PopStyleVar(2);
	//	ImGui::End();
	//	return;
	//}

	//auto pos = ImGui::GetCurrentWindowRead()->Pos;
	//auto size = ImGui::GetCurrentWindowRead()->Size;
	//auto contentSize = ImGui::GetCurrentWindowRead()->ContentSize;
	//PRINT(Str("Scene Dock pos : %.2f, %.2f | size : %.2f, %.2f | content size : %.2f, %.2f", pos.x, pos.y, size.x, size.y, contentSize.x, contentSize.y));

	//{
	//	// Set viewport rect is size with scene dock rect
	//	EntityType e_type;
	//	switch (Engine::GetEngineMode())
	//	{
	//	case Engine::ENGINE_MODE::EDITING:
	//		e_type = (EntityType)EngineEntityType::EDITOR_CAMERA;
	//		break;
	//	default:
	//		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Engine mode is not valid!");
	//	}

	//	auto camera = GameWorld::GetCurrent()->GetTheOnlyEntityWithType(e_type);
	//	auto cam = GameWorld::GetCurrent()->GetComponent<PerspectiveCameraCom>(camera)->GetCamera();
	//	cam->SetViewPort(Vec2u(pos.x, pos.y), Vec2u(size.x, size.y));
	//	cam->cameraSettings.aspectRatioWbyH = float(size.x) / float(size.y);

	//	auto editorPickingComSys = static_cast<EditorPickingComSys*>(GameWorld::GetCurrent()->GetComponentSystem("EditorPickingComSys"));
	//	editorPickingComSys->SetSceneDockDrawList(ImGui::GetWindowDrawList());
	//}

	//{
	//	// Display final frame buffer to scene dock, notice the uv is y inverted due to OpenGl definition
	//	auto buffer = Renderer3D::s_Data.gpuBuffer.CurrentFinalFrameBuffer;
	//	ImGui::Image(reinterpret_cast<ImTextureID>(buffer->GetRenderTargetID()), size, ImVec2(0, 1), ImVec2(1, 0));
	//}
	//{
	//	// Mouse over scene dock should not be captured by ImGui
	//	bool isWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && !ImGui::IsAnyItemHovered();
	//	if (isWindowHovered)
	//	{
	//		ImGuiUtil::IgnoreMouseCaptured = ImGuiUtil::IgnoreKeyBoardCaptured = true;
	//	}
	//}
	//manager->PopWidgetStyle();
	//ImGui::PopStyleVar(2);
	//ImGui::End();
}
