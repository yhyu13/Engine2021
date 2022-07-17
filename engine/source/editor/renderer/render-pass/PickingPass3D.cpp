#include "engine-precompiled-header.h"
#include "PickingPass3D.h"
#include "engine/ecs/header/header.h"
#include "editor/ui/common/BaseEngineWidgetManager.h"
#include "editor/ui/common/widgets/SceneHierarchyDock.h"

#include <imgui/addons/ImGuizmo/ImGuizmo.h>

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
        m_renderTarget = FrameBuffer::Create(texture_dim, texture_dim, FrameBuffer::BUFFER_FORMAT::UINT_RGBA8);

        auto rm = ResourceManager<Scene3DNode>::GetInstance();
        m_particlePickingMesh = rm->TryGet("unit_sphere")->Get()->Copy();
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
    }
    {
        switch (Renderer3D::s_Data.RENDER_MODE)
        {
        case Renderer3D::RENDER_MODE::CANONICAL:
            m_pickingShader = Shader::Create("$shader:picking_entity_id_shader.vert",
                                             "$shader:picking_entity_id_shader.frag");
            break;
        case Renderer3D::RENDER_MODE::MULTIDRAW:
            m_pickingShader = Shader::Create("$shader:picking_entity_id_shader_MultiDraw.vert",
                                             "$shader:picking_entity_id_shader_MultiDraw.frag");
            break;
        }
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
        // We must unbind here such that things would not render to this render target
        m_renderTarget->Unbind();
        // Interpret data
        uint32_t id;
        if (GetPickedResult(&id))
        {
            DEBUG_PRINT(
                "Get picking result : " + Str(id) + " data : " + Str((uint32_t)m_blitData[0]) + " " + Str((uint32_t)
                    m_blitData[1])
                + " " + Str((uint32_t)m_blitData[2]) + " " + Str((uint32_t)m_blitData[3]));
            if (m_pickedEntity = m_parentWorld->GetEntityFromID(id); m_pickedEntity.Valid())
            {
                auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
                auto sceneDock = static_cast<SceneHierarchyDock*>(manager->GetWidget("SceneHierarchyDock"));
                sceneDock->SetSelectionMaskExclusion(m_pickedEntity, true,
                                                     !InputManager::GetInstance()->IsKeyPressed(KEY_LEFT_SHIFT));
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
        bool bUIHover = !ImGui::IsAnyItemHovered();
        bool bNotIsOverGuizmo = !ImGuizmo::IsOver();
        if (auto input = InputManager::GetInstance();
            input->IsMouseButtonTriggered(MOUSE_BUTTON_LEFT) && bUIHover && bNotIsOverGuizmo)
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
            auto cam = m_parentWorld->GetComponent<PerspectiveCameraCom>(camera)->GetCamera();

            auto cursor_pos = input->GetCursorPositionXY();

            if (IN_RANGE(cursor_pos.x, (float)cam->cameraSettings.viewportOrigin.x,
                         (float)cam->cameraSettings.viewportOrigin.x + (float)cam->cameraSettings.viewportSize.x)
                && IN_RANGE(cursor_pos.y, (float)cam->cameraSettings.viewportOrigin.y,
                            (float)cam->cameraSettings.viewportOrigin.y + (float)cam->cameraSettings.viewportSize.y))
            {
                Vec3f out_eye_world;
                Vec3f out_dir_wprld;
                cam->GenerateRayFromCursorSpace(cursor_pos, true, true, out_eye_world, out_dir_wprld);
                Vec3f look_at_world = out_eye_world + out_dir_wprld;

                m_pickingCam.type = PerspectiveCameraType::FIRST_PERSON;
                m_pickingCam.SetProjection(1.0 * DEG2RAD, 1.0, cam->cameraSettings.nearZ, cam->cameraSettings.farZ);
                m_pickingCam.SetLookAt(out_eye_world, look_at_world);
                m_pickingCam.OnUpdate();

                Render();
                m_shouldRead = true;
            }
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
    Renderer3D::s_Data.RENDER_PASS = Renderer3D::RENDER_PASS::CUSTOM;
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

    LongMarch_ForEach(
        [this](const Renderer3D::RenderObj_CPU& renderObj)
        {
            auto scene = renderObj.entity.GetComponent<Scene3DCom>();
            auto body = renderObj.entity.GetComponent<Body3DCom>();
            if (body.Valid())
            {
                if (const auto& bv = body->GetBoundingVolume(); bv)
                {
                    if (DistanceCullingTest(bv))
                    {
                        scene->SetShouldDraw(false, false);
                    }
                    else if (ViewFustrumCullingTest(bv))
                    {
                        scene->SetShouldDraw(false, false);
                    }
                }
            }
            scene->Draw(m_drawBind);
        }
        , {
            &Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_OPAQUE,
            &Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_TRANSPARENT
        });

    // Bind multi draw func
    Renderer3D::s_Data.multiDrawBuffer.multiDrawRenderPassCallback.submitCallback = &m_submitBatchBind;
    Renderer3D::s_Data.multiDrawBuffer.multiDrawRenderPassCallback.clearCallback = &m_clearBatchBind;
    Renderer3D::CommitBatchRendering();
}

void longmarch::PickingPass::UpdateShader()
{
    const auto& shader = m_pickingShader;
    shader->Bind();
    Mat4 pv;
    if (Renderer3D::s_Data.enable_reverse_z)
    {
        pv = m_pickingCam.GetReverseZViewProjectionMatrix();
    }
    else
    {
        pv = m_pickingCam.GetViewProjectionMatrix();
    }
    shader->SetMat4("u_PVMatrix", pv);
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
    const auto& shader = m_pickingShader;
    shader->Bind();
    switch (Renderer3D::s_Data.RENDER_MODE)
    {
    case Renderer3D::RENDER_MODE::MULTIDRAW:
        {
            if (m_multiDraw_PickingTr.size() >= max_batch)
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
