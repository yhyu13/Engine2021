#include "engine-precompiled-header.h"
#include "Particle3DComSys.h"
#include "engine/ecs/components/3d/Transform3DCom.h"

longmarch::Particle3DComSys::Particle3DComSys()
{
	m_systemSignature.AddComponent<Particle3DCom>();
}

void longmarch::Particle3DComSys::Update(double dt)
{
	EARLY_RETURN(dt);

	EntityType e_type;
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::EDITING: case Engine::ENGINE_MODE::INGAME_EDITING:
		e_type = (EntityType)EngineEntityType::EDITOR_CAMERA;
		break;
	case Engine::ENGINE_MODE::INGAME:
		e_type = (EntityType)EngineEntityType::PLAYER_CAMERA;
		break;
	case Engine::ENGINE_MODE::INGAME_FREEROAM:
		e_type = (EntityType)EngineEntityType::FREEROAM_CAMERA;
		break;
	default:
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Engine mode is not valid!");
	}

	const auto& player = GameWorld::GetCurrent()->GetTheOnlyEntityWithType(e_type);

	auto camera = GameWorld::GetCurrent()->GetComponent<PerspectiveCameraCom>(player)->GetCamera();
	auto cameraPosition = camera->GetWorldPosition();

	ForEach(
		[dt, cameraPosition](EntityDecorator e)
	{
		auto particleCom = e.GetComponent<Particle3DCom>();
		particleCom->SetCenter(e.GetComponent<Transform3DCom>()->GetGlobalPos());
		particleCom->Update(dt, cameraPosition);
	}
	);
}

void longmarch::Particle3DComSys::RenderParticleSystems(const PerspectiveCamera* camera)
{
	Sort(camera);
	ForEach(
		[camera](EntityDecorator e)
	{
		e.GetComponent<Particle3DCom>()->RenderParticleSystems(camera);
	}
	);
}

void longmarch::Particle3DComSys::Sort(const PerspectiveCamera* camera)
{
	struct RenderParticleObj
	{
		explicit RenderParticleObj(const EntityDecorator& e, float d)
			:
			obj(e),
			distance(d)
		{}
		EntityDecorator obj;
		float distance;
	};
	struct RenderParticleObj_ComparatorLesser // used in priority queue that puts objects in greater distances at front
	{
		bool operator()(const RenderParticleObj& lhs, const RenderParticleObj& rhs) noexcept
		{
			return lhs.distance < rhs.distance;
		}
	};

	std::priority_queue<RenderParticleObj, LongMarch_Vector<RenderParticleObj>, RenderParticleObj_ComparatorLesser> depth_sorted_translucent_obj;
	Mat4 pv = camera->GetViewProjectionMatrix();
	ForEach(
		[&pv, &depth_sorted_translucent_obj](EntityDecorator e)
	{
		auto pos = e.GetComponent<Transform3DCom>()->GetGlobalPos();
		auto ndc_pos = pv * Vec4f(pos, 1.0f);
		depth_sorted_translucent_obj.emplace(e, ndc_pos.z);
	}
	);

	RemoveAllEntities();
	while (!depth_sorted_translucent_obj.empty())
	{
		auto translucent_renderObj = depth_sorted_translucent_obj.top();
		AddEntity(translucent_renderObj.obj);
		depth_sorted_translucent_obj.pop();
	}
}
