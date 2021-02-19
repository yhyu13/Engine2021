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

	const auto& player = GameWorld::GetCurrent()->GetTheOnlyEntityWithType(e_type);

	auto camera = GameWorld::GetCurrent()->GetComponent<PerspectiveCameraCom>(player)->GetCamera();

	ForEach(
		[dt, camera](EntityDecorator e)
	{
		auto particleCom = e.GetComponent<Particle3DCom>();
		particleCom->SetCenter(e.GetComponent<Transform3DCom>()->GetGlobalPos());
		particleCom->Update(dt, camera);
	}
	);
}
