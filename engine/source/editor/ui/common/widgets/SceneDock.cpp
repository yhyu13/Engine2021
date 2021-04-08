#include "engine-precompiled-header.h"
#include "SceneDock.h"
#include "../BaseEngineWidgetManager.h"
#include "engine/ecs/header/header.h"
#include "engine/renderer/Renderer3D.h"
#include "editor/ecs/header/header.h"

#define USE_SCENE_DOCK

longmarch::SceneDock::SceneDock()
{
	m_IsVisible = true;
	m_Size = ScaleSize({ 800, 600 });
}

void longmarch::SceneDock::Render()
{
#ifdef USE_SCENE_DOCK
	WIDGET_EARLY_QUIT();

	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();
	ImVec2 windowsize = ImVec2(GetWindowSize_X(), GetWindowSize_Y());
	ImVec2 mainMenuWindowSize = PosScaleBySize(m_Size, windowsize);
	ImGui::SetNextWindowSize(mainMenuWindowSize, ImGuiCond_Once);

	// Broderless and no padding, result in a clean capture of the whole scene
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_Border, ImGuiUtil::ColGreen);

	if (!ImGui::Begin("Scene", &m_IsVisible, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav))
	{
		// Early out if the window is collapsed, as an optimization.
		manager->PopWidgetStyle();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(1);
		ImGui::End();
		return;
	}
	auto input = InputManager::GetInstance();
	auto cursor_pos = input->GetCursorPositionXY();
	const auto& prop = Engine::GetWindow()->GetWindowProperties();
	auto pos = ImGui::GetCurrentWindowRead()->InnerRect.Min; // In screen space (not in window space)
	auto size = ImGui::GetCurrentWindowRead()->InnerRect.GetSize();
	//DEBUG_PRINT(Str("Window pos : %d, %d Scene Dock pos : %.2f, %.2f | size : %.2f, %.2f Cursor Pos %.2f, %.2f", prop.m_xpos, prop.m_ypos, pos.x, pos.y, size.x, size.y, cursor_pos.x, cursor_pos.y));

	{
		// Set viewport rect is size with scene dock rect
		EntityType e_type;
		switch (Engine::GetEngineMode())
		{
		case Engine::ENGINE_MODE::EDITING:
			e_type = (EntityType)EngineEntityType::EDITOR_CAMERA;
			break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Engine mode is not valid!");
		}

		auto camera = GameWorld::GetCurrent()->GetTheOnlyEntityWithType(e_type);
		auto cam = GameWorld::GetCurrent()->GetComponent<PerspectiveCameraCom>(camera)->GetCamera();

		cam->SetViewPort(Vec2u(pos.x - prop.m_xpos, pos.y - prop.m_ypos), Vec2u(size.x, size.y));
		cam->cameraSettings.aspectRatioWbyH = float(size.x) / float(size.y);

		auto editorPickingComSys = static_cast<EditorPickingComSys*>(GameWorld::GetCurrent()->GetComponentSystem("EditorPickingComSys"));
		editorPickingComSys->SetSceneDockDrawList(ImGui::GetWindowDrawList());
	}

	{
		// Display final frame buffer to scene dock, notice the uv is y inverted due to OpenGl definition
		auto buffer = Renderer3D::s_Data.gpuBuffer.CurrentFinalFrameBuffer;
		ImGui::Image(reinterpret_cast<ImTextureID>(buffer->GetRenderTargetID()), size, ImVec2(0, 1), ImVec2(1, 0));
	}
	
	{
		// Mouse over scene dock should not be captured by ImGui
		switch (Engine::GetWindow()->GetCursorMode())
		{
		case Window::CURSOR_MODE::HIDDEN_AND_FREE:
			ImGuiUtil::IgnoreMouseCaptured = ImGuiUtil::IgnoreKeyBoardCaptured = true;
			break;
		default:
			break;
		}
	}

	manager->PopWidgetStyle();
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(1);
	ImGui::End();
#else
	/********************************
* Scene Dock suffers from a wired bug that when reaching negative x values.
* Turn it off temporarily.
********************************/
	{
		EntityType e_type;
		switch (Engine::GetEngineMode())
		{
		case Engine::ENGINE_MODE::EDITING:
			e_type = (EntityType)EngineEntityType::EDITOR_CAMERA;
			break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Engine mode is not valid!");
		}
		auto camera = GameWorld::GetCurrent()->GetTheOnlyEntityWithType(e_type);
		auto cam = GameWorld::GetCurrent()->GetComponent<PerspectiveCameraCom>(camera)->GetCamera();
		const auto& prop = Engine::GetWindow()->GetWindowProperties();
		cam->SetViewPort(Vec2u(0), Vec2u(prop.m_width, prop.m_height));
		if (prop.IsResizable)
		{
			cam->cameraSettings.aspectRatioWbyH = float(prop.m_width) / float(prop.m_height);
		}
	}
#endif // !USE_SCENE_DOCK
}
