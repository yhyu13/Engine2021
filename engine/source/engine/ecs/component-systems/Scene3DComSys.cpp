#include "engine-precompiled-header.h"
#include "Scene3DComSys.h"
#include "engine/ecs/header/header.h"
#include "engine/core/thread/StealThreadPool.h"

longmarch::Scene3DComSys::Scene3DComSys()
{
	m_systemSignature.AddComponent<Transform3DCom>();
	m_systemSignature.AddComponent<Scene3DCom>();
	m_systemSignature.AddComponent<Body3DCom>();
}

void longmarch::Scene3DComSys::PreRenderUpdate(double dt)
{
	/**************************************************************
	*	Update light buffer
	**************************************************************/
	{
		ENG_TIME("Prepare Light");
		const auto& lights = m_parentWorld->GetAllEntityWithType({ (EntityType)EngineEntityType::DIRECTIONAL_LIGHT,
																	(EntityType)EngineEntityType::POINT_LIGHT,
																	(EntityType)EngineEntityType::SPOT_LIGHT });
		auto& lightBuffer = Renderer3D::s_Data.cpuBuffer.LIGHTS_BUFFERED;
		lightBuffer.clear();
		for (uint32_t i = 0; i < lights.size(); ++i)
		{
			auto light = lights[i];

			auto trans = GetComponent<Transform3DCom>(light);
			auto scene = GetComponent<Scene3DCom>(light);
			auto lightCom = GetComponent<LightCom>(light);

			if (trans.Valid() && scene.Valid() && lightCom.Valid())
			{
				bool emissive = false;
				auto sceneDataRef = scene->GetSceneData(false);
				if (sceneDataRef)
				{
					const auto& sceneData = *(sceneDataRef);
					for (const auto& [level, data] : sceneData)
					{
						if (auto& mat = data->material; mat->emissive)
						{
							emissive = true;
							auto currentLight = Renderer3D::LightBuffer_CPU();
							currentLight.type = light.m_type - (EntityType)EngineEntityType::LIGHT_OBJ_BEGIN;
							currentLight.thisLight = light;
							currentLight.Kd = mat->Kd;
							currentLight.Pos = trans->GetGlobalPos();
							currentLight.Direction = trans->GetGlobalRot();
							currentLight.Attenuation = lightCom->attenuation;
							lightBuffer.emplace_back(std::move(currentLight));
							break;
						}
					}
					if (!emissive || sceneData.empty())
					{
						throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Lights need at least one emssive material!");
					}
				}
			}
		}
	}

	/**************************************************************
	*	Prepare Scene data
	**************************************************************/
	{
		ENG_TIME("Prepare Scene");
		PrepareScene(dt);
	}

	/**************************************************************
	*	Sort translucent object by depth
	**************************************************************/
	{
		struct RenderTransparentObj_CPU
		{
			explicit RenderTransparentObj_CPU(const Renderer3D::RenderObj_CPU& e, float d)
				:
				obj(e),
				distance(d)
			{}
			Renderer3D::RenderObj_CPU obj;
			float distance;
		};
		struct RenderTransparentObj_CPU_ComparatorLesser // used in priority queue that puts objects in greater distances at front
		{
			bool operator()(const RenderTransparentObj_CPU& lhs, const RenderTransparentObj_CPU& rhs) noexcept
			{
				return lhs.distance < rhs.distance;
			}
		};

		EntityType e_type;
		switch (Engine::GetEngineMode())
		{
		case Engine::ENGINE_MODE::EDITING:
			e_type = (EntityType)EngineEntityType::EDITOR_CAMERA;
			break; 
		case Engine::ENGINE_MODE::INGAME:
			e_type = (EntityType)EngineEntityType::PLAYER_CAMERA;
			break;
		default:
			return;
		}

		auto camera = m_parentWorld->GetTheOnlyEntityWithType(e_type);
		auto camera_ptr = m_parentWorld->GetComponent<PerspectiveCameraCom>(camera)->GetCamera();

		std::priority_queue<RenderTransparentObj_CPU, LongMarch_Vector<RenderTransparentObj_CPU>, RenderTransparentObj_CPU_ComparatorLesser> depth_sorted_translucent_obj;
		Mat4 pv = camera_ptr->GetViewProjectionMatrix();
		for (auto& renderObj : Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_TRANSPARENT)
		{
			auto pos = renderObj.entity.GetComponent<Transform3DCom>()->GetGlobalPos();
			auto ndc_pos = pv * Vec4f(pos, 1.0f);
			depth_sorted_translucent_obj.emplace(renderObj, ndc_pos.z);
		}
		Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_TRANSPARENT.clear();
		while (!depth_sorted_translucent_obj.empty())
		{
			auto translucent_renderObj = depth_sorted_translucent_obj.top();
			Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_TRANSPARENT.push_back(translucent_renderObj.obj);
			depth_sorted_translucent_obj.pop();
		}
	}
}

void longmarch::Scene3DComSys::Render()
{
	// Debug draw view frustum test bounding volume for each renderable objects
	if (m_enableDebugDraw)
	{
		LongMarch_ForEach(
			[](const Renderer3D::RenderObj_CPU& renderObj)
		{
			auto body = renderObj.entity.GetComponent<Body3DCom>();
			if (body.Valid())
			{
				if (auto bv = body->GetBoundingVolume(); bv)
				{
					bv->RenderShape();
				}
			}
		}
		, { Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_OPAQUE, 
			Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_TRANSPARENT});
	}
}

void longmarch::Scene3DComSys::PrepareScene(double dt)
{
	Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_OPAQUE.clear();
	Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_TRANSPARENT.clear();

	auto root = m_parentWorld->GetTheOnlyEntityWithType((EntityType)(EngineEntityType::SCENE_ROOT));
	{
		auto parentTrCom = GetComponent<Transform3DCom>(root).GetPtr();
		// Reset scene root transform component
		parentTrCom->Reset();
		const auto& children = GetComponent<ChildrenCom>(root)->GetChildren();
		for (const auto& child : children)
		{
			bool active = GetComponent<ActiveCom>(child)->IsActive();
			auto trans = GetComponent<Transform3DCom>(child);
			auto scene = GetComponent<Scene3DCom>(child);

			trans->SetParentModelTr(parentTrCom->GetSuccessionModelTr(trans));
			scene->SetShouldDraw(active, true);
			if (scene->IsVisible())
			{
				LOCK_GUARD_NC();
				if (scene->IsTranslucenctRenderType() || scene->IsParticleRenderType()) [[unlikely]]
				{
					Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_TRANSPARENT.emplace_back(EntityDecorator{ child, m_parentWorld });
				}
				else [[likely]]
				{
					Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_OPAQUE.emplace_back(EntityDecorator{ child, m_parentWorld });
				}
			}
			if (auto childrenCom = GetComponent<ChildrenCom>(child).GetPtr(); !childrenCom->IsLeaf())
			{
#ifdef MULTITHREAD_PRE_RENDER_UPDATE
				m_threadJob.push(std::move(StealThreadPool::GetInstance()->enqueue_task([this, dt, child, trans, childrenCom]() {RecursivePrepareScene(dt, child, trans, childrenCom, 0); }).share()));
#else
				RecursivePrepareScene(dt, child, trans, childrenCom, 0);
#endif
			}
		}
	}
#ifdef MULTITHREAD_PRE_RENDER_UPDATE
	while (!m_threadJob.empty())
	{
		m_threadJob.front().wait();
		m_threadJob.pop();
	}
#endif
}

void longmarch::Scene3DComSys::RecursivePrepareScene(double dt, const Entity& parent, Transform3DCom* parentTr, ChildrenCom* parentChildrenCom, unsigned int level)
{
	const auto& children = parentChildrenCom->GetChildren();
	for (const auto& child : children)
	{
		bool active = GetComponent<ActiveCom>(child)->IsActive();
		auto trans = GetComponent<Transform3DCom>(child);
		auto scene = GetComponent<Scene3DCom>(child);

		trans->SetParentModelTr(parentTr->GetSuccessionModelTr(trans));
		scene->SetShouldDraw(active, true);
		if (scene->IsVisible())
		{
			LOCK_GUARD_NC();
			if (scene->IsTranslucenctRenderType() || scene->IsParticleRenderType()) [[unlikely]]
			{
				Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_TRANSPARENT.emplace_back(EntityDecorator{ child, m_parentWorld });
			}
			else [[likely]]
			{
				Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_OPAQUE.emplace_back(EntityDecorator{ child, m_parentWorld });
			}
		}
		if (auto childrenCom = GetComponent<ChildrenCom>(child).GetPtr(); !childrenCom->IsLeaf())
		{
#ifdef MULTITHREAD_PRE_RENDER_UPDATE
			m_threadJob.push(std::move(StealThreadPool::GetInstance()->enqueue_task([this, dt, child, trans, childrenCom, level]() {RecursivePrepareScene(dt, child, trans, childrenCom, level + 1); }).share()));
#else
			RecursivePrepareScene(dt, child, trans, childrenCom, level + 1);
#endif
		}
	}
}

void longmarch::Scene3DComSys::RenderOpaqueObj()
{
	for (auto& renderObj : Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_OPAQUE)
	{
		RenderWithModeOpaque(renderObj);
	}
}

void longmarch::Scene3DComSys::RenderTransparentObj()
{
	for (auto& renderObj : Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_TRANSPARENT)
	{
		RenderWithModeTransparent(renderObj);
	}
}

void longmarch::Scene3DComSys::SetVFCullingParam(bool enable, const ViewFrustum& VF, const Mat4& WorldSpaceToViewFrustum)
{
	m_vfcParam.enableVFCulling = enable;
	m_vfcParam.VFinViewSpace = VF;
	m_vfcParam.WorldSpaceToViewSpace = WorldSpaceToViewFrustum;
}

void longmarch::Scene3DComSys::SetDistanceCullingParam(bool enable, const Vec3f& center, float Near, float Far)
{
	m_distanceCParam.enableDistanceCulling = enable;
	m_distanceCParam.center = center;
	m_distanceCParam.Near = Near;
	m_distanceCParam.Far = Far;
}

void longmarch::Scene3DComSys::SetRenderShaderName(const std::string& shaderName)
{
	m_RenderShaderName = shaderName;
}

void longmarch::Scene3DComSys::RenderWithModeOpaque(Renderer3D::RenderObj_CPU& renderObj)
{
	auto scene = renderObj.entity.GetComponent<Scene3DCom>();
	auto body = renderObj.entity.GetComponent<Body3DCom>();

	scene->SetShaderName(m_RenderShaderName);
	bool hasBody = body.Valid();

	if (hasBody)
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
	switch (Renderer3D::s_Data.RENDER_PASS)
	{
	case Renderer3D::RENDER_PASS::SCENE:
	{
		if (!scene->IsHideInGame())
		{
			scene->Draw();
		}
	}
	break;
	case Renderer3D::RENDER_PASS::SHADOW:
	{
		if (!scene->IsHideInGame() && scene->IsCastShadow())
		{
			scene->Draw();
		}
	}
	break;
	}
}


void longmarch::Scene3DComSys::RenderWithModeTransparent(Renderer3D::RenderObj_CPU& renderObj)
{
	auto particle = renderObj.entity.GetComponent<Particle3DCom>();
	auto scene = renderObj.entity.GetComponent<Scene3DCom>();
	auto body = renderObj.entity.GetComponent<Body3DCom>();

	scene->SetShaderName(m_RenderShaderName);
	bool isParticle = particle.Valid() && scene->IsParticleRenderType();

	bool hasBody = body.Valid();
	if (hasBody)
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
			if (isParticle)
			{
				particle->SetRendering(scene->GetShouldDraw());
			}
		}
	}
	switch (Renderer3D::s_Data.RENDER_PASS)
	{
	case Renderer3D::RENDER_PASS::SCENE:
	{
		if (!scene->IsHideInGame())
		{
			if (isParticle)
			{
				particle->PrepareDrawDataWithViewMatrix(m_vfcParam.WorldSpaceToViewSpace);
				scene->Draw(particle.GetPtr());
			}
			else
			{
				scene->Draw();
			}
		}
	}
	break;
	case Renderer3D::RENDER_PASS::SHADOW:
	{
		if (!scene->IsHideInGame() && scene->IsCastShadow())
		{
			if (isParticle)
			{
				particle->PrepareDrawDataWithViewMatrix(m_vfcParam.WorldSpaceToViewSpace);
				scene->Draw(particle.GetPtr());
			}
			else
			{
				scene->Draw();
			}
		}
	}
	break;
	}
}

void longmarch::Scene3DComSys::RenderWithModeParticle(Renderer3D::RenderObj_CPU& renderObj)
{
}

bool longmarch::Scene3DComSys::ViewFustrumCullingTest(const std::shared_ptr<Shape>& BoudingVolume)
{
	if (!m_vfcParam.enableVFCulling)
	{
		return false;
	}
	else
	{
		return BoudingVolume->VFCTest(m_vfcParam.VFinViewSpace, m_vfcParam.WorldSpaceToViewSpace);
	}
}

bool longmarch::Scene3DComSys::DistanceCullingTest(const std::shared_ptr<Shape>& BoudingVolume)
{
	if (!m_distanceCParam.enableDistanceCulling)
	{
		return false;
	}
	else
	{
		return BoudingVolume->DistanceTest(m_distanceCParam.center, m_distanceCParam.Near, m_distanceCParam.Far);
	}
}