#include "engine-precompiled-header.h"
#include "PickingPass3D.h"
#include "engine/ecs/header/header.h"
#include "editor/ui/BaseEngineWidgetManager.h"
#include "editor/ui/widgets/SceneHierarchyDock.h"

void longmarch::PickingPass::Init()
{
	// Create picking texture and framebuffer
	{
		Texture::Setting setting;
		setting.width = texture_dim;
		setting.height = texture_dim;
		setting.channels = 4;
		setting.has_mipmap = false;
		setting.linear_filter = false;
		setting.float_type = false;
		m_renderTexture = Texture2D::Create(setting);
		m_renderTarget = FrameBuffer::Create(texture_dim, texture_dim, FrameBuffer::BUFFER_FORMAT::UINT8);
	}
	// Create picking gpu buffer
	{
		m_multiDraw_ssbo_PickingModelTrsBuffer = ShaderStorageBuffer::Create(nullptr, 0);
		m_multiDraw_ssbo_PickingEntityIdsBuffer = ShaderStorageBuffer::Create(nullptr, 0);
	}
	// Create multidraw bindings
	{
		m_drawBind = std::move(std::bind(&PickingPass::Draw, this, std::placeholders::_1));
		m_submitBatchBind = std::move(std::bind(&PickingPass::SubmitBatch, this));
		m_clearBatchBind = std::move(std::bind(&PickingPass::ClearBatch, this));
		m_camera = &m_pickingCam;
	}
}

void longmarch::PickingPass::BeginRenderPass()
{
	// Read the result of the picking request in beginning of the next frame
	if (m_shouldRead)
	{
		// Reset data
		*(uint32_t*)m_blitData &= 0u;
		// Bind FBO before read pixel
		m_renderTarget->Bind();
		m_renderTexture->ReadTexture(m_blitData);
		// We must unbind here such that further rendering like UI would work
		m_renderTarget->Unbind();
		// Interpret data
		if (uint32_t id; GetPickedResult(&id))
		{
			DEBUG_PRINT("Get picking result : " + Str(id) + " data : " + Str((uint32_t)m_blitData[0]) + " " + Str((uint32_t)m_blitData[1])
				+ " " + Str((uint32_t)m_blitData[2]) + " " + Str((uint32_t)m_blitData[3]));
			if (m_pickedEntity = m_parentWorld->GetEntityFromID(id); m_pickedEntity.Valid())
			{
				auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
				auto sceneDock = static_cast<SceneHierarchyDock*>(manager->GetWidget("SceneHierarchyDock"));
				sceneDock->SetSelectionMaskExclusion(m_pickedEntity, true, !InputManager::GetInstance()->IsKeyPressed(KEY_LEFT_SHIFT));
			}
		}
		else
		{
			m_pickedEntity = Entity();
		}
	}
	{
		// Process new picking request
		m_shouldRead = false;
		// Push picking rendering cmd on LMB in the editor viewport
		bool capture = !ImGuiUtil::IsMouseCaptured();
		if (auto input = InputManager::GetInstance();
			input->IsMouseButtonTriggered(MOUSE_BUTTON_LEFT) && capture)
		{
			EntityType e_type;
			switch (Engine::GetEngineMode())
			{
			case Engine::ENGINE_MODE::EDITING: case Engine::ENGINE_MODE::INGAME_EDITING:
				e_type = (EntityType)EngineEntityType::EDITOR_CAMERA;
				break;
			default:
				return;
			}

			auto camera = m_parentWorld->GetTheOnlyEntityWithType(e_type);
			auto current_camera = m_parentWorld->GetComponent<PerspectiveCameraCom>(camera)->GetCamera();

			auto cursor_pos = input->GetCursorPositionXY();
			Vec3f pick_eye;
			Vec3f pick_at;
			Vec3f pick_dir;

			current_camera->GenerateRayFromScreenSpace(cursor_pos, true, true, pick_eye, pick_dir);
			pick_at = pick_eye + pick_dir;

			m_pickingCam.type = PerspectiveCameraType::FIRST_PERSON;
			m_pickingCam.SetProjection(1.0 * DEG2RAD, 1.0, current_camera->cameraSettings.nearZ, current_camera->cameraSettings.farZ);
			m_pickingCam.SetLookAt(pick_eye, pick_at);
			m_pickingCam.OnUpdate();

			Render();

			m_shouldRead = true;
			// Render the debug obj
			//scene->SetVisiable(true);
			//trans->SetGlobalPos(pick_eye + 10.0f * pick_dir);
		}
	}
}

void longmarch::PickingPass::EndRenderPass()
{
	m_renderTarget->Unbind();
	Renderer3D::s_Data.multiDrawBuffer.multiDrawRenderPassCallback.submitCallback = nullptr;
	Renderer3D::s_Data.multiDrawBuffer.multiDrawRenderPassCallback.clearCallback = nullptr;
}

Entity longmarch::PickingPass::GetPickedEntity() const
{
	return m_pickedEntity;
}

bool longmarch::PickingPass::GetPickedResult(uint32_t* id) const
{
	if (uint32_t result = *(uint32_t*)m_blitData;
		result != 0u && result != ~0u)
	{
		*id = result;
		return true;
	}
	else
	{
		return false;
	}
}

void longmarch::PickingPass::Render()
{
	// Config render settings
	Renderer3D::s_Data.RENDER_PASS = Renderer3D::RENDER_PASS::EMPTY;
	RenderCommand::PolyModeFill();
	RenderCommand::Blend(false);
	RenderCommand::DepthTest(true, true);
	RenderCommand::CullFace(true, false);

	// Set background to all ones for testing
	RenderCommand::SetViewport(0, 0, texture_dim, texture_dim);
	RenderCommand::SetClearColor(Vec4f(1, 1, 1, 1));
	m_renderTarget->Bind();
	m_renderTexture->AttachToFrameBuffer();
	RenderCommand::Clear();

	// Update uniform shader variables
	UpdateShader();

	// Calling base functions
	SetVFCullingParam(true, m_pickingCam.GetViewFrustumInViewSpace(), m_pickingCam.GetViewMatrix());
	SetDistanceCullingParam(false, Vec3f(), 0, 0);
	RenderWithCulling();

	// Bind multi draw func
	Renderer3D::s_Data.multiDrawBuffer.multiDrawRenderPassCallback.submitCallback = &m_submitBatchBind;
	Renderer3D::s_Data.multiDrawBuffer.multiDrawRenderPassCallback.clearCallback = &m_clearBatchBind;
	Renderer3D::CommitBatchRendering();
}

void longmarch::PickingPass::UpdateShader()
{
	if (!m_shader)
	{
		auto shaderName = "PickingSystemShader";
		auto it = Renderer3D::s_Data.ShaderMap.find(shaderName);
		if (it == Renderer3D::s_Data.ShaderMap.end())
		{
			ENGINE_EXCEPT(L"Shader called " + str2wstr(shaderName) + L" has not been managed!");
		}
		m_shader = it->second;
	}
	{
		Mat4 pv;
		if (Renderer3D::s_Data.enable_reverse_z)
		{
			pv = m_camera->GetReverseZViewProjectionMatrix();
		}
		else
		{
			pv = m_camera->GetViewProjectionMatrix();
		}
		m_shader->Bind();
		m_shader->SetMat4("u_PVMatrix", pv);
	}
}

void longmarch::PickingPass::SubmitBatch()
{
	{
		auto ptr = &(m_multiDraw_PickingTr[0]);
		auto size = m_multiDraw_PickingTr.size() * sizeof(Mat4);
		m_multiDraw_ssbo_PickingModelTrsBuffer->UpdateBufferData(ptr, size);
		m_multiDraw_ssbo_PickingModelTrsBuffer->Bind(1); // TODO use serialized binding location
	}
	{
		auto ptr = &(m_multiDraw_PickingEntityId[0]);
		auto size = m_multiDraw_PickingEntityId.size() * sizeof(int);
		m_multiDraw_ssbo_PickingEntityIdsBuffer->UpdateBufferData(ptr, size);
		m_multiDraw_ssbo_PickingEntityIdsBuffer->Bind(2); // TODO use serialized binding location
	}
}

void longmarch::PickingPass::ClearBatch()
{
	m_multiDraw_PickingEntityId.clear();
	m_multiDraw_PickingTr.clear();
}

void longmarch::PickingPass::RenderOne(Renderer3D::RenderObj_CPU& renderObj)
{
	auto scene = renderObj.entity.GetComponent<Scene3DCom>();
	scene->Draw(m_drawBind);
}

void longmarch::PickingPass::Draw(const Renderer3D::RenderData_CPU& data)
{
	switch (Renderer3D::s_Data.RENDER_MODE)
	{
	case Renderer3D::RENDER_MODE::MULTIDRAW:
	{
		if (m_multiDraw_PickingTr.size() >= m_max_batch)
		{
			Renderer3D::CommitBatchRendering();
		}
		m_multiDraw_PickingEntityId.emplace_back(data.entity.m_id);
		m_multiDraw_PickingTr.emplace_back(data.transform);
		Renderer3D::Draw(data);
	}
	break;
	case Renderer3D::RENDER_MODE::CANONICAL:
	{
		Renderer3D::s_Data.CurrentShader = m_shader;
		Renderer3D::s_Data.CurrentShader->SetInt("u_EntityId", data.entity.m_id);
		Renderer3D::s_Data.CurrentShader->SetMat4("u_ModleTr", data.transform);
		data.mesh->Draw();
	}
	break;
	}
}