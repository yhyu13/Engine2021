#include "engine-precompiled-header.h"
#include "EditorPickingComSys.h"
#include "engine/ecs/header/header.h"
#include "editor/ui/BaseEngineWidgetManager.h"
#include "engine/math/Geommath.h"

#include <imgui/addons/ImGuizmo/ImGuizmo.h>

//#define USE_IMGUIZMO_CAM

void longmarch::EditorPickingComSys::Init()
{
	m_pickingPass.SetParentWorld(m_parentWorld);
	m_pickingPass.Init();
	m_outlinePass.SetParentWorld(m_parentWorld);
	m_outlinePass.Init();
}

void longmarch::EditorPickingComSys::Render()
{
	m_pickingPass.BeginRenderPass();
	m_pickingPass.EndRenderPass();
}

void longmarch::EditorPickingComSys::Render2()
{
	m_outlinePass.BeginRenderPass();
	m_outlinePass.EndRenderPass();
}

void longmarch::EditorPickingComSys::RenderUI()
{
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::EDITING:
		ImGuizmo::BeginFrame();
		{
			if (auto e = m_pickingPass.GetPickedEntity(); e.Valid())
			{
				auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
				if (auto es = manager->GetAllSelectedEntityBuffered(); es.empty())
				{
					manager->PushBackSelectedEntityBuffered(e);
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
		break;
	}
}

void longmarch::EditorPickingComSys::ManipulatePickedEntityGizmos(const Entity& e)
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
	auto current_camera = GetComponent<PerspectiveCameraCom>(camera)->GetCamera();
	auto trans = GetComponent<Transform3DCom>(e);
	if (camera != e && trans.Valid())
	{
		const auto& prop = Engine::GetWindow()->GetWindowProperties();
		ImGuizmo::SetRect(prop.m_upperleft_xpos, prop.m_upperleft_ypos, prop.m_width, prop.m_height);

		auto trans_mat = trans->GetModelTr();
		Mat4 cam_view = current_camera->GetViewMatrix();
		Mat4 cam_proj = current_camera->GetProjectionMatrix();
#ifdef USE_IMGUIZMO_CAM
		auto cam_location = current_camera->GetWorldPosition();
		auto cam_rotation = current_camera->GetGlobalRotation();
		auto cam_lookat = cam_location + cam_rotation * Geommath::WorldFront;
		ImGuizmo::LookAt(&(cam_location[0]), &(cam_lookat[0]), &(Vec3f(0, 0, 1)[0]), &(cam_view[0][0]));
		ImGuizmo::Perspective(current_camera->cameraSettings.fovy_rad * 0.5 * RAD2DEG, current_camera->cameraSettings.aspectRatioWbyH, current_camera->cameraSettings.nearZ, current_camera->cameraSettings.farZ, &(cam_proj[0][0]));
#endif // USE_IMGUIZMO_CAM

		// Draw guizmo
		if (ImGuizmo::Manipulate(&(cam_view[0][0]), &(cam_proj[0][0]), operation, mode, &(trans_mat[0][0]), nullptr, nullptr, nullptr, nullptr))
		{
			trans->SetModelTr(trans_mat);
		}
	}
}