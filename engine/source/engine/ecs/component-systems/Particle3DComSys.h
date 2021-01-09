#pragma once

#include "engine/ecs/BaseComponentSystem.h"
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/ComponentDecorator.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/components/3d/Particle3DCom.h"
#include "engine/ecs/components/PerspectiveCameraCom.h"
#include "engine/Engine.h"

namespace AAAAgames
{
	class Particle3DComSys final : public BaseComponentSystem
	{
	public:
		NONCOPYABLE(Particle3DComSys);
		COMSYS_DEFAULT_COPY(Particle3DComSys);

		Particle3DComSys()
		{
			m_systemSignature.AddComponent<Particle3DCom>();
		}

		virtual void Update(double dt) override
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

			const auto& players = GameWorld::GetCurrent()->GetAllEntityWithType(e_type);
			if (players.size() != 1)
			{
				throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Player camera does not exist or there is more than one player!");
			}
			const auto& player = players[0];

			auto camera = GameWorld::GetCurrent()->GetComponent<PerspectiveCameraCom>(player)->GetCamera();
			auto worldPosition = camera->GetWorldPosition();
			glm::vec3 cameraPosition(worldPosition.x, worldPosition.y, worldPosition.z);

			ForEach(
				[dt, cameraPosition](EntityDecorator e)
				{
					e.GetComponent<Particle3DCom>()->Update(dt, cameraPosition);
				}
			);
		}

		void RenderParticleSystems(PerspectiveCamera* camera)
		{
			ForEach(
				[camera](EntityDecorator e)
				{
					e.GetComponent<Particle3DCom>()->RenderParticleSystems(camera);
				}
			);
		}

		inline void SetRenderShaderName(const std::string& shaderName)
		{
			m_RenderShaderName = shaderName;
		}

	private:

		std::string m_RenderShaderName = { "Default" };
	};
}
