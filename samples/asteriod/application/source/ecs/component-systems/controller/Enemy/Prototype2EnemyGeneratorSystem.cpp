#include "application-precompiled-header.h"
#include "Prototype2EnemyGeneratorSystem.h"
#include "engine/Engine.h"
#include "engine/ecs/header/header.h"
#include "ecs/EntityType.h"
#include "ecs/components/AIControllerCom.h"

void longmarch::Prototype2EnemyGeneratorSystem::PostRenderUpdate(double dt)
{
	// Only start when INGAME mode
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::INGAME:
		break;
	default:
		return;
	}

	if (m_parentWorld == GameWorld::GetCurrent())
	{
		// Generate an enemy on the fly after every 2 seconds to attack the player
		if (timer.Mark() >= 2.0f) // time greater than or equal 2 seconds
		{
			count = RAND_I(1, 4);
			timer.Reset();
		}
		if (count-- > 0)
		{
			auto player = m_parentWorld->GetTheOnlyEntityWithType((EntityType)GameEntityType::PLAYER);
			auto player_pos = m_parentWorld->GetComponent<Transform3DCom>(player)->GetGlobalPos();

			for (int i = 0; i < count; ++i)
			{
				float theta = RAND_F(-1.f, 1.0f) * PI2;
				float dist = RAND_F(13.f, 20.0f);
				float x = (float)(dist * std::cosf(theta));
				float y = (float)(dist * std::sinf(theta));

				// Creating an entity
				auto root_1 = m_parentWorld->GenerateEntity3D((EntityType)(GameEntityType::ENEMY_ASTERIOD), true, true);

				auto body = root_1.GetComponent<Body3DCom>();
				body->m_bodyInfo.type = RBType::dynamicBody;
				body->m_bodyInfo.colliderDimensionExtent = 0.5;

				// Creating transform of the given entity
				auto trans = root_1.GetComponent<Transform3DCom>();
				trans->SetGlobalPos(Vec3f(x, y, 0) + player_pos); // To randomly generate in circles around the origin
				trans->SetLocalScale(Vec3f(RAND_F(0.5f, 3.0f), RAND_F(0.5f, 3.0f), RAND_F(0.5f, 3.0f))); // Let it have some size
				trans->SetGlobalRot(Geommath::ToQuaternion(Vec3f(RAND_F(-180, 180), RAND_F(-180, 180), RAND_F(-180, 180)) * DEG2RAD));
				trans->SetGlobalRotVel(Vec3f(RAND_F(-10, 10), RAND_F(-10, 10), RAND_F(-10, 10)));

				auto scene = root_1.GetComponent<Scene3DCom>();
				scene->SetVisiable(true);

				// Mesh's material data
				auto rm = ResourceManager<Scene3DNode>::GetInstance();
				auto enemy_cube = rm->TryGet("003_asteroid_rock")->Get()->Copy();
				enemy_cube->ModifyAllMaterial([&](Material* material) {
					material->Kd = Vec3f(RAND_F(0.65f, 1.0f));
					material->roughness = 1.0f;
					material->metallic = .5f;
					material->emissive = false;
				});
				scene->SetSceneData(enemy_cube);

				// Adding AIControllerCom to the root entity and fetching it
				AIControllerCom aiCom(root_1);
				// Setting up BT and Blackboard
				aiCom.SetBT(("$asset:BehaviorTrees/MoveToPlayer.json"));
				m_parentWorld->AddComponent<AIControllerCom>(root_1.GetEntity(), aiCom);

			}
		}
	}
}
