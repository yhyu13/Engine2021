#include "engine-precompiled-header.h"
#include "Scene3DComSys.h"
#include "engine/ecs/header/header.h"
#include "engine/core/thread/StealThreadPool.h"

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
			trans->SetParentModelTr(parentTrCom->GetSuccessionModelTr(trans));
			auto scene = GetComponent<Scene3DCom>(child);
			scene->SetShouldDraw(active, true);
			if (scene->IsVisible())
			{
				LOCK_GUARD_NC();
				if (!scene->IsTranslucenctRendering())
				{
					Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_OPAQUE.emplace_back(EntityDecorator{ child,m_parentWorld });
				}
				else
				{
					Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_TRANSPARENT.emplace_back(EntityDecorator{ child,m_parentWorld });
				}
			}
			if (auto childrenCom = GetComponent<ChildrenCom>(child).GetPtr(); !childrenCom->IsLeaf())
			{
#if MULTITHREAD_PRE_RENDER_UPDATE == 0
				RecursivePrepareScene(dt, child, trans, childrenCom, 0);
#else
				m_threadJob.push(std::move(StealThreadPool::GetInstance()->enqueue_task([this, dt, child, trans, childrenCom]() {RecursivePrepareScene(dt, child, trans, childrenCom, 0); }).share()));
#endif
			}
		}
	}
#if MULTITHREAD_PRE_RENDER_UPDATE
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
		trans->SetParentModelTr(parentTr->GetSuccessionModelTr(trans));
		auto scene = GetComponent<Scene3DCom>(child);
		scene->SetShouldDraw(active, true);
		if (scene->IsVisible())
		{
			LOCK_GUARD_NC();
			if (!scene->IsTranslucenctRendering())
			{
				Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_OPAQUE.emplace_back(EntityDecorator{ child,m_parentWorld });
			}
			else
			{
				Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_TRANSPARENT.emplace_back(EntityDecorator{ child,m_parentWorld });
			}
		}
		if (auto childrenCom = GetComponent<ChildrenCom>(child).GetPtr(); !childrenCom->IsLeaf())
		{
#if MULTITHREAD_PRE_RENDER_UPDATE == 0
			RecursivePrepareScene(dt, child, trans, childrenCom, level + 1);
#else
			m_threadJob.push(std::move(StealThreadPool::GetInstance()->enqueue_task([this, dt, child, trans, childrenCom, level]() {RecursivePrepareScene(dt, child, trans, childrenCom, level + 1); }).share()));
#endif
		}
	}
}

void longmarch::Scene3DComSys::RenderOpaqueObj()
{
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::INGAME:
		for (auto& renderObj : Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_OPAQUE)
		{
			RenderWithModeInGame(renderObj);
		}
		break;
	default:
		for (auto& renderObj : Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_OPAQUE)
		{
			RenderWithModeEditor(renderObj);
		}
		break;
	}
}

void longmarch::Scene3DComSys::RenderTranslucentObj(const PerspectiveCamera* camera)
{
	std::priority_queue<Renderer3D::RenderTranslucentObj_CPU, 
		LongMarch_Vector<Renderer3D::RenderTranslucentObj_CPU>, 
		Renderer3D::RenderTranslucentObj_CPU_ComparatorLesser> depth_sorted_translucent_obj;

	Mat4 pv = camera->GetViewProjectionMatrix();
	for (auto& renderObj : Renderer3D::s_Data.cpuBuffer.RENDERABLE_OBJ_TRANSPARENT)
	{
		auto trans = renderObj.entity.GetComponent<Transform3DCom>();
		auto pos = trans->GetGlobalPos();
		auto ndc_pos = pv * Vec4f(pos, 1.0f);
		depth_sorted_translucent_obj.emplace(renderObj, ndc_pos.z);
	}

	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::INGAME:
		while (!depth_sorted_translucent_obj.empty())
		{
			auto translucent_renderObj = depth_sorted_translucent_obj.top();
			RenderWithModeInGame(translucent_renderObj.obj);
			depth_sorted_translucent_obj.pop();
		}
		break;
	default:
		while (!depth_sorted_translucent_obj.empty())
		{
			auto translucent_renderObj = depth_sorted_translucent_obj.top();
			RenderWithModeEditor(translucent_renderObj.obj);
			depth_sorted_translucent_obj.pop();
		}
		break;
	}
}

void longmarch::Scene3DComSys::RenderWithModeEditor(Renderer3D::RenderObj_CPU& renderObj)
{
	{
		auto scene = renderObj.entity.GetComponent<Scene3DCom>();
		scene->SetShaderName(m_RenderShaderName);
		auto body = renderObj.entity.GetComponent<Body3DCom>();
		if (body.Valid())
		{
			if (const auto& bv = body->GetBV(); bv)
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
		{
			switch (m_RenderMode)
			{
			case RenderMode::SCENE:
			{
				scene->Draw();
			}
			return;
			case RenderMode::SCENE_AND_BBOX:
			{
				// TODO add toggle of visualizing bounding volume in imgui of body component
				/*if (body.Valid() && body->GetBV())
				{
					body->GetBV()->RenderShape();
				}*/
				scene->Draw();
			}
			return;
			case RenderMode::SHADOW:
			{
				if (scene->IsCastShadow())
				{
					scene->Draw();
				}
			}
			return;
			}
		}
	}
}


void longmarch::Scene3DComSys::RenderWithModeInGame(Renderer3D::RenderObj_CPU& renderObj)
{
	{
		auto scene = renderObj.entity.GetComponent<Scene3DCom>();
		scene->SetShaderName(m_RenderShaderName);
		auto body = renderObj.entity.GetComponent<Body3DCom>();
		if (body.Valid())
		{
			if (const auto& bv = body->GetBV(); bv)
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
		{
			switch (m_RenderMode)
			{
			case RenderMode::SCENE:
			case RenderMode::SCENE_AND_BBOX:
			{
				if (!scene->IsHideInGame())
				{
					scene->Draw();
				}
			}
			return;
			case RenderMode::SHADOW:
			{
				if (scene->IsCastShadow())
				{
					scene->Draw();
				}
			}
			return;
			}
		}
	}
}