#include "engine-precompiled-header.h"
#include "EditorPickingComSys.h"
#include "engine/ecs/header/header.h"
#include "editor/ui/BaseEngineWidgetManager.h"
#include "editor/ui/widgets/SceneHierarchyDock.h"
#include "engine/math/Geommath.h"

#include <imgui/addons/ImGuizmo/ImGuizmo.h>

#define USE_IMGUIZMO_CAM 1

void longmarch::EditorPickingComSys::Init()
{
	m_renderPass.SetParentWorld(m_parentWorld);
	m_renderPass.Init();
	OutlineFrameBuffer = FrameBuffer::Create(1, 1, FrameBuffer::BUFFER_FORMAT::Float16);
}

void longmarch::EditorPickingComSys::Render()
{
	m_renderPass.BeginRenderPass();
	m_renderPass.EndRenderPass();
}

void longmarch::EditorPickingComSys::Render2()
{
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::EDITING:
		{
			// TODO : render stencil based outline rendering
			if (OutlineFrameBuffer->GetBufferSize() != Renderer3D::s_Data.window_size)
			{
				OutlineFrameBuffer = FrameBuffer::Create(Renderer3D::s_Data.window_size.x, Renderer3D::s_Data.window_size.y, FrameBuffer::BUFFER_FORMAT::Float16);
			}

			auto cam_type = (EntityType)EngineEntityType::EDITOR_CAMERA;
			auto camera = m_parentWorld->GetTheOnlyEntityWithType(cam_type);
			auto cam = m_parentWorld->GetComponent<PerspectiveCameraCom>(camera)->GetCamera();

			auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
			auto es = manager->GetAllSelectedEntityBuffered();
			if (es.empty())
			{
				break;
			}

			auto render_pipe_original = Renderer3D::s_Data.RENDER_PIPE;
			auto render_mode_original = Renderer3D::s_Data.RENDER_MODE;
			// Config render settings
			Renderer3D::s_Data.RENDER_PASS = Renderer3D::RENDER_PASS::SHADOW;
			Renderer3D::s_Data.RENDER_PIPE = Renderer3D::RENDER_PIPE::FORWARD;
			Renderer3D::s_Data.RENDER_MODE = Renderer3D::RENDER_MODE::CANONICAL;

			RenderCommand::PolyModeFill();
			RenderCommand::Blend(false);
			RenderCommand::StencilTest(true, true);
			RenderCommand::DepthTest(true, true);
			RenderCommand::CullFace(true, false);

			// Set background to all ones for testing
			Vec2u traget_resoluation = OutlineFrameBuffer->GetBufferSize();
			RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);
			RenderCommand::SetClearColor(Vec4f(0, 0, 0, 0));
			OutlineFrameBuffer->Bind();
			RenderCommand::Clear();

			// 1. Draw objects to stencil
			RenderCommand::StencilFunc(longmarch::RendererAPI::CompareEnum::ALWAYS);

			auto shader_name = "ShadowBuffer";
			Renderer3D::s_Data.CurrentShader = Renderer3D::s_Data.ShaderMap[shader_name];
			Renderer3D::s_Data.CurrentShader->Bind();
			Renderer3D::s_Data.CurrentShader->SetMat4("u_PVMatrix", cam->GetReverseZViewProjectionMatrix());
			m_parentWorld->ForEach(
				es,
				[&shader_name, &cam](EntityDecorator e)
			{ 
				auto particle = e.GetComponent<Particle3DCom>();
				bool isParticle = particle.Valid();

				auto scene = e.GetComponent<Scene3DCom>();
				scene->SetShaderName(shader_name);	
				scene->SetShouldDraw(true, true);

				if (!scene->IsHideInGame() && scene->IsCastShadow())
				{
					if (isParticle)
					{
						/*particle->SetRendering(scene->GetShouldDraw());
						particle->PrepareDrawWithViewMatrix(cam->GetViewMatrix());
						scene->Draw(particle.GetPtr());*/
					}
					else
					{
						scene->Draw();
					}
				}
			}
			);

			// 2. Transfer stencil buffer to default frame buffer
			int default_framebuffer_rendererID = 0;
			RenderCommand::TransferStencilBit(
				OutlineFrameBuffer->GetRendererID(),
				OutlineFrameBuffer->GetBufferSize().x,
				OutlineFrameBuffer->GetBufferSize().y,

				default_framebuffer_rendererID,
				Renderer3D::s_Data.window_size.x,
				Renderer3D::s_Data.window_size.y
			);

			// 3. Draw slighly larger objects with outline color to default frame buffer
			RenderCommand::DepthTest(false, false);
			RenderCommand::StencilTest(true, false);
			RenderCommand::StencilFunc(longmarch::RendererAPI::CompareEnum::NEQUAL);

			RenderCommand::BindDefaultFrameBuffer();
			traget_resoluation = Renderer3D::s_Data.window_size;
			RenderCommand::SetViewport(0, 0, traget_resoluation.x, traget_resoluation.y);

			shader_name = "OutlineShader";
			Renderer3D::s_Data.CurrentShader = Renderer3D::s_Data.ShaderMap[shader_name];
			Renderer3D::s_Data.CurrentShader->Bind();
			Renderer3D::s_Data.CurrentShader->SetMat4("u_PVMatrix", cam->GetReverseZViewProjectionMatrix());
			m_parentWorld->ForEach(
				es,
				[&shader_name, &cam](EntityDecorator e)
			{
				// Up scale
				auto trans = e.GetComponent<Transform3DCom>();
				auto orig_scale = trans->GetLocalScale();
				trans->SetLocalScale(orig_scale * 1.05f);

				auto particle = e.GetComponent<Particle3DCom>();
				bool isParticle = particle.Valid();

				auto scene = e.GetComponent<Scene3DCom>();
				scene->SetShaderName(shader_name);
				scene->SetShouldDraw(true, true);

				if (!scene->IsHideInGame() && scene->IsCastShadow())
				{
					if (isParticle)
					{
						/*particle->SetRendering(scene->GetShouldDraw());
						particle->PrepareDrawWithViewMatrix(cam->GetViewMatrix());
						scene->Draw(particle.GetPtr());*/
					}
					else
					{
						scene->Draw();
					}
				}

				// Reset to original scale
				trans->SetLocalScale(orig_scale);
			}
			);

			RenderCommand::StencilTest(false, false);
			Renderer3D::s_Data.RENDER_PIPE = render_pipe_original;
			Renderer3D::s_Data.RENDER_MODE = render_mode_original;
		}
		break;
	}


}

void longmarch::EditorPickingComSys::RenderUI()
{
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::EDITING:
		ImGuizmo::BeginFrame();
		{
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
#if USE_IMGUIZMO_CAM == 1
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