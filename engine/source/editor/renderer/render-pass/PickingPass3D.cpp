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
		m_drawBind_Particle = std::move(std::bind(&PickingPass::DrawParticle, this, std::placeholders::_1));
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
			case Engine::ENGINE_MODE::EDITING:
				e_type = (EntityType)EngineEntityType::EDITOR_CAMERA;
				break;
			default:
				return;
			}

			auto camera = m_parentWorld->GetTheOnlyEntityWithType(e_type);
			auto camera_ptr = m_parentWorld->GetComponent<PerspectiveCameraCom>(camera)->GetCamera();

			auto cursor_pos = input->GetCursorPositionXY();
			Vec3f pick_eye;
			Vec3f pick_at;
			Vec3f pick_dir;

			camera_ptr->GenerateRayFromScreenSpace(cursor_pos, true, true, pick_eye, pick_dir);
			pick_at = pick_eye + pick_dir;

			m_pickingCam.type = PerspectiveCameraType::FIRST_PERSON;
			m_pickingCam.SetProjection(1.0 * DEG2RAD, 1.0, camera_ptr->cameraSettings.nearZ, camera_ptr->cameraSettings.farZ);
			m_pickingCam.SetLookAt(pick_eye, pick_at);
			m_pickingCam.OnUpdate();

			Render();

			m_shouldRead = true;
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
	RenderWithCullingTest();

	// Bind multi draw func
	Renderer3D::s_Data.multiDrawBuffer.multiDrawRenderPassCallback.submitCallback = &m_submitBatchBind;
	Renderer3D::s_Data.multiDrawBuffer.multiDrawRenderPassCallback.clearCallback = &m_clearBatchBind;
	Renderer3D::CommitBatchRendering();
}

void longmarch::PickingPass::UpdateShader()
{
	{
		auto shader = Renderer3D::s_Data.ShaderMap["PickingSystemShader"];
		shader->Bind();
		Mat4 pv;
		if (Renderer3D::s_Data.enable_reverse_z)
		{
			pv = m_camera->GetReverseZViewProjectionMatrix();
		}
		else
		{
			pv = m_camera->GetViewProjectionMatrix();
		}
		shader->SetMat4("u_PVMatrix", pv);
	}
	{
		auto shader = Renderer3D::s_Data.ShaderMap["PickingSystemShader_Particle"];
		shader->Bind();
		Mat4 p;
		if (Renderer3D::s_Data.enable_reverse_z)
		{
			p = m_camera->GetReverseZProjectionMatrix();
		}
		else
		{
			p = m_camera->GetProjectionMatrix();
		}
		shader->SetMat4("u_ProjectionMatrix", p);
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

void longmarch::PickingPass::Draw(const Renderer3D::RenderData_CPU& data)
{
	auto shader = Renderer3D::s_Data.ShaderMap["PickingSystemShader"];
	shader->Bind();
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
		shader->SetInt("u_EntityId", data.entity.m_id);
		shader->SetMat4("u_ModleTr", data.transform);
		Renderer3D::Draw(data);
	}
	break;
	}
}

void longmarch::PickingPass::DrawParticle(const Renderer3D::ParticleInstanceDrawData& data)
{
	auto shader = Renderer3D::s_Data.ShaderMap["PickingSystemShader_Particle"];
	shader->Bind();
	for (auto& [texture, instanceData] : data)
	{
		texture->BindTexture(0);
		shader->SetFloat("rows", instanceData.textureRows);
		shader->SetInt("u_EntityId", instanceData.entity.m_id);

		int pointer = 0;
		float* data = new float[instanceData.models.size() * Renderer3D::PARTICLE_INSTANCED_DATA_LENGTH];
		for (size_t i = 0; i < instanceData.models.size(); ++i)
		{
			const auto& matrix = instanceData.models[i];
			data[pointer++] = matrix[0][0];
			data[pointer++] = matrix[0][1];
			data[pointer++] = matrix[0][2];
			data[pointer++] = matrix[0][3];
			data[pointer++] = matrix[1][0];
			data[pointer++] = matrix[1][1];
			data[pointer++] = matrix[1][2];
			data[pointer++] = matrix[1][3];
			data[pointer++] = matrix[2][0];
			data[pointer++] = matrix[2][1];
			data[pointer++] = matrix[2][2];
			data[pointer++] = matrix[2][3];
			data[pointer++] = matrix[3][0];
			data[pointer++] = matrix[3][1];
			data[pointer++] = matrix[3][2];
			data[pointer++] = matrix[3][3];

			const Vec4f& offsets = instanceData.textureOffsets[i];
			data[pointer++] = offsets.x;
			data[pointer++] = offsets.y;
			data[pointer++] = offsets.z;
			data[pointer++] = offsets.w;

			const float blendFactor = instanceData.blendFactors[i];
			data[pointer++] = blendFactor;
		}
		Renderer3D::DrawParticlesInstance(data, instanceData.models.size());
		delete[] data;
	}
}