#include "engine-precompiled-header.h"
#include "EditorPickingComSys.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/EntityType.h"
#include "engine/ecs/components/PerspectiveCameraCom.h"
#include "editor/ui/BaseEngineWidgetManager.h"
#include "editor/ui/widgets/SceneHierarchyDock.h"
#include "engine/math/Geommath.h"

#include <imgui/addons/ImGuizmo/ImGuizmo.h>

void AAAAgames::EditorPickingComSys::Init()
{
	m_renderPass.SetParentWorld(m_parentWorld);
	m_renderPass.Init();
}

void AAAAgames::EditorPickingComSys::RenderUI()
{
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::EDITING: case Engine::ENGINE_MODE::INGAME_EDITING:
		ImGuizmo::BeginFrame();
		{
			m_renderPass.BeginRenderPass();
			m_renderPass.EndRenderPass();
			if (auto e = m_renderPass.GetPickedEntity(); e.Valid())
			{
				auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
				if (auto es = manager->GetAllSelectedEntityBuffered(); es.empty())
				{
					manager->PushBackSelectedEntity(e);
					ManipulatePickedEntityGizmos(e);
				}
				else if (es.size() == 1)
				{
					auto entity_0 = *es.begin();
					ManipulatePickedEntityGizmos(entity_0);
				}
				else
				{
					// TODO : consider multi-selection
				}
			}
		}
		ImGuizmo::EndFrame();
		break;
	}
}

void AAAAgames::EditorPickingComSys::ManipulatePickedEntityGizmos(const Entity& e)
{
	static ImGuizmo::OPERATION operation; // shared for all game worlds in editing
	static ImGuizmo::MODE mode; // shared for all game worlds in editing
	static Vec3f snap;

	if (!InputManager::GetInstance()->IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !ImGui::IsAnyItemActive() && !ImGuizmo::IsUsing())
	{
		if (InputManager::GetInstance()->IsKeyTriggered(KEY_W))
		{
			operation = ImGuizmo::OPERATION::TRANSLATE;
			DEBUG_PRINT("Switch to translate mode");
		}
		if (InputManager::GetInstance()->IsKeyTriggered(KEY_E))
		{
			operation = ImGuizmo::OPERATION::ROTATE;
			DEBUG_PRINT("Switch to rotation mode");
		}
		if (InputManager::GetInstance()->IsKeyTriggered(KEY_R))
		{
			operation = ImGuizmo::OPERATION::SCALE;
			mode = ImGuizmo::MODE::LOCAL;
			DEBUG_PRINT("Switch to local scale mode");
		}
		if (InputManager::GetInstance()->IsKeyTriggered(KEY_T))
		{
			mode = ImGuizmo::MODE::LOCAL;
			DEBUG_PRINT("Switch to local mode");
		}
		if (InputManager::GetInstance()->IsKeyTriggered(KEY_Y) && operation != ImGuizmo::OPERATION::SCALE)
		{
			mode = ImGuizmo::MODE::WORLD;
			DEBUG_PRINT("Switch to world mode");
		}
	}

	switch (operation)
	{
	case ImGuizmo::TRANSLATE:
		snap = Vec3f(0.25); // TODO implement command for changing snap value
		break;
	case ImGuizmo::ROTATE:
		snap = Vec3f(0.5); // TODO implement command for changing snap value
		break;
	case ImGuizmo::SCALE:
		snap = Vec3f(0.05); // TODO implement command for changing snap value
		break;
	}

	auto camera = m_parentWorld->GetTheOnlyEntityWithType((EntityType)EngineEntityType::EDITOR_CAMERA);
	auto current_camera = m_parentWorld->GetComponent<PerspectiveCameraCom>(camera)->GetCamera();
	auto trans = GetComponent<Transform3DCom>(e);
	if (camera != e && trans.Valid())
	{
		ImGuizmo::SetRect(0, 0, Window::width, Window::height);

		Mat4 cam_view = current_camera->GetViewMatrix();
		Mat4 cam_proj = Geommath::ReverseZProjectionMatrixZeroOne(current_camera->cameraSettings.fovy_rad,
			current_camera->cameraSettings.aspectRatioWbyH,
			current_camera->cameraSettings.nearZ,
			current_camera->cameraSettings.farZ);
		auto trans_mat = trans->GetModelTr();

		ImGuizmo::SetDrawlist();
		if (ImGuizmo::Manipulate(&(cam_view[0][0]), &(cam_proj[0][0]), operation, mode, &trans_mat[0][0], nullptr, nullptr))
		{
			trans->SetModelTr(trans_mat);
		}
	}
}