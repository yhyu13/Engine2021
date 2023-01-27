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
	case Engine::ENGINE_MODE::EDITING: 
		e_type = (EntityType)EngineEntityType::EDITOR_CAMERA;
		break;
	case Engine::ENGINE_MODE::INGAME:
		e_type = (EntityType)EngineEntityType::PLAYER_CAMERA;
		break;
	default:
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Engine mode is not valid!");
	}

	auto world = GameWorld::GetCurrent();
	const auto& player = world->GetTheOnlyEntityWithType(e_type);
	auto camera = world->GetComponent<PerspectiveCameraCom>(player)->GetCamera();

	auto jobHandle = ParEachChunk(
		[dt, camera](const EntityChunkContext& e)
		{
			const auto particleComs = e.GetComponentPtr<Particle3DCom>();
			const auto transform3DComs = e.GetComponentPtr<Transform3DCom>();
	
			for (auto i = e.BeginIndex(); i <= e.EndIndex(); ++i)
			{
				const auto particleCom = particleComs + i;
				const auto transform3DCom = transform3DComs + i;
				particleCom->SetCenter(transform3DCom->GetGlobalPos());
				particleCom->Update(dt, camera);
			}
		}
	).share();

	m_perInvokationPhaseJobQueue[EInvokationPhase::LATE_UPDATE].push(
		[jobHandle = std::move(jobHandle)]()
		{
			jobHandle.wait();
		}
	);
}

void Particle3DComSys::LateUpdate(double dt)
{
	auto& jobQueue = m_perInvokationPhaseJobQueue[EInvokationPhase::LATE_UPDATE];
	while(!jobQueue.empty())
	{
		jobQueue.pop_front()();
	}
}


