/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: GAM550
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 10/25/2020
- End Header ----------------------------*/

#include "application-precompiled-header.h"
#include "Prototype2PlayerArsenalComSys.h"
#include "engine/ecs/header/header.h"
#include "ecs/EntityType.h"
#include "events/CustomEvents.h"
#include "ecs/components/AIControllerCom.h"

void longmarch::Prototype2PlayerArsenalComSys::Init()
{
	{
		auto queue = EventQueue<Prototype2EventType>::GetInstance();
		ManageEventSubHandle(queue->Subscribe<Prototype2PlayerArsenalComSys>(this, Prototype2EventType::GEN_PROJECTILE, &Prototype2PlayerArsenalComSys::_ON_GEN_PROJECTILE));
		ManageEventSubHandle(queue->Subscribe<Prototype2PlayerArsenalComSys>(this, Prototype2EventType::GEN_SPACE_SHIP, &Prototype2PlayerArsenalComSys::_ON_GEN_SPACE_SHIP));
	} 
	{
		auto queue = EventQueue<EngineEventType>::GetInstance();
		ManageEventSubHandle(queue->Subscribe<Prototype2PlayerArsenalComSys>(this, EngineEventType::COLLISION, &Prototype2PlayerArsenalComSys::_ON_PROJECTILE_COLLISION));
	}
}

void longmarch::Prototype2PlayerArsenalComSys::Update(double dt)
{
	{
		m_projectileFireDeltaTime += dt;
	}
	for (auto iter = m_projectileDatas.begin(); iter != m_projectileDatas.end();)
	{
		auto& data = *(iter);
		if ((data.lifeTime -= dt) <= 0.f)
		{
			auto queue = EventQueue<EngineEventType>::GetInstance();
			auto e1 = MemoryManager::Make_shared<EngineGCEvent>(EntityDecorator{ data.m_this, m_parentWorld });
			m_GC.push_back(e1);
			iter = m_projectileDatas.erase(iter);
		}
		else
		{
			++iter;
		}
	}
	if (m_spaceshipDatas.size() > m_MaxNumDrone)
	{
		// Remove extra spaceships
		{
			LongMarch_Vector<SpaceshipData> data{ m_spaceshipDatas.begin(), m_spaceshipDatas.end() - m_MaxNumDrone};
			for (auto iter = data.begin(); iter != data.end(); ++iter)
			{
				auto& data = *(iter);
				auto queue = EventQueue<EngineEventType>::GetInstance();
				auto e1 = MemoryManager::Make_shared<EngineGCEvent>(EntityDecorator{ data.m_this, m_parentWorld });
				m_GC.push_back(e1);
			}
		}
		{
			LongMarch_Vector<SpaceshipData> data{ m_spaceshipDatas.end() - m_MaxNumDrone, m_spaceshipDatas.end() };
			m_spaceshipDatas = data;
		}
	}
	for (auto iter = m_spaceshipDatas.begin(); iter != m_spaceshipDatas.end();)
	{
		auto& data = *(iter);
		if ((data.lifeTime -= dt) <= 0.f)
		{
			auto queue = EventQueue<EngineEventType>::GetInstance();
			auto e1 = MemoryManager::Make_shared<EngineGCEvent>(EntityDecorator{ data.m_this, m_parentWorld });
			m_GC.push_back(e1);
			iter = m_spaceshipDatas.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void  longmarch::Prototype2PlayerArsenalComSys::PostRenderUpdate(double dt)
{
	auto queue = EventQueue<EngineEventType>::GetInstance();
	for (auto event : m_GC)
	{
		queue->Publish(event);
	}
	m_GC.clear();
}

void longmarch::Prototype2PlayerArsenalComSys::_ON_GEN_PROJECTILE(EventQueue<Prototype2EventType>::EventPtr e)
{
	if (m_parentWorld == GameWorld::GetCurrent())
	{
		// This function shows an example of using dynamic cast to have one event callback handling multiple event types
		if (auto event = std::dynamic_pointer_cast<Prototype2PlayerGenerateProjectile>(e))
		{
			if (m_projectileFireDeltaTime < (1.0f / m_MaxProjectileFireRate))
			{
				return;
			}
			m_projectileFireDeltaTime = 0.f;

			auto player_orientation = event->m_player_orientation;
			LongMarch_Vector<Quaternion> projectile_orientation{ player_orientation }; 
			auto opposite = Quaternion(Geommath::RotationMat(Geommath::ROT_AXIS::YAW, PI)) * player_orientation;
			switch (m_MaxNumProjectile)
			{
			case 2:
				projectile_orientation.push_back(Quaternion(opposite));
				break;
			case 3:
				projectile_orientation.push_back(Quaternion(Geommath::RotationMat(Geommath::ROT_AXIS::YAW, PI / 6.f)) * opposite);
				projectile_orientation.push_back(Quaternion(Geommath::RotationMat(Geommath::ROT_AXIS::YAW, -PI / 6.f)) * opposite);
				break;
			case 4:
				projectile_orientation.push_back(opposite);
				projectile_orientation.push_back(Quaternion(Geommath::RotationMat(Geommath::ROT_AXIS::YAW, PI / 6.f)) * player_orientation);
				projectile_orientation.push_back(Quaternion(Geommath::RotationMat(Geommath::ROT_AXIS::YAW, -PI / 6.f)) * player_orientation);
				break;
			default:
				break;
			}

			for (int i = 0; i < m_MaxNumProjectile; ++i)
			{
				auto e_type = (EntityType)GameEntityType::PROJECTILE;
				auto entity = m_parentWorld->GenerateEntity3D(e_type, true, true);
				auto onwer = m_parentWorld->GetTheOnlyEntityWithType((EntityType)GameEntityType::PLAYER);
				m_projectileDatas.emplace_back(ProjectileData{ .m_this = entity, .owner = onwer, .lifeTime = 5.0f });
				{
					auto rm = ResourceManager<Scene3DNode>::GetInstance();
					auto meshName = "projectile_0";
					auto mesh = rm->TryGet(meshName)->Get()->Copy();
					auto scene = m_parentWorld->GetComponent<Scene3DCom>(entity);
					scene->SetVisiable(true);
					mesh->ModifyAllMaterial([&](Material* material)
					{
						material->Kd = Vec3f(0.54, 1.0, 0.);
					});
					scene->SetSceneData(mesh);
				}
				{
					auto body_owner = m_parentWorld->GetComponent<Body3DCom>(onwer);
					auto player_radius = body_owner->m_body->GetShape()->GetRadius();
					auto velocity_vector = projectile_orientation[i] * Geommath::WorldFront;
					auto trans = m_parentWorld->GetComponent<Transform3DCom>(entity);
					trans->SetLocalScale(Vec3f(0.5));
					trans->SetGlobalPos(event->m_player_position + velocity_vector * player_radius);
					trans->SetGlobalRot(projectile_orientation[i]);
					trans->SetGlobalVel(velocity_vector * 10.0f);
				}
				{
					auto body = m_parentWorld->GetComponent<Body3DCom>(entity);
					body->m_bodyInfo.type = RBType::dynamicBody;
					body->m_bodyInfo.colliderDimensionExtent = 0.5;
					body->m_bodyInfo.entityTypeIngoreSet.emplace((EntityType)GameEntityType::PLAYER);
				}
				{
					auto camera = m_parentWorld->GetTheOnlyEntityWithType((EntityType)EngineEntityType::PLAYER_CAMERA);
					auto trans = GetComponent<Transform3DCom>(camera);
					AudioManager::GetInstance()->PlaySoundByName("sfx-lazer-beam_0", AudioVector3(trans->GetGlobalPos()), -10, 1);
				}
			}
		}
		else if (auto event = std::dynamic_pointer_cast<Prototype2SpaceShipGenerateProjectile>(e))
		{
			if (event->m_entity.GetWorld() == m_parentWorld)
			{
				auto e_type = (EntityType)GameEntityType::PROJECTILE;
				auto entity = m_parentWorld->GenerateEntity3D(e_type, true, true);
				auto onwer = event->m_entity;
				m_projectileDatas.emplace_back(ProjectileData{ .m_this = entity, .owner = onwer, .lifeTime = 2.0f });
				{
					auto rm = ResourceManager<Scene3DNode>::GetInstance();
					auto meshName = "projectile_0";
					auto mesh = rm->TryGet(meshName)->Get()->Copy();
					auto scene = m_parentWorld->GetComponent<Scene3DCom>(entity);
					scene->SetVisiable(true);
					mesh->ModifyAllMaterial([&](Material* material)
					{
						material->Kd = Vec3f(0.0, 0.96, 1.);
					});
					scene->SetSceneData(mesh);
				}
				{
					auto body_owner = m_parentWorld->GetComponent<Body3DCom>(onwer);
					//auto player_radius = body_owner->m_body->GetShape()->GetRadius(); // The spaceship currently does not have collision
					auto player_radius = 1.0f;
					auto velocity_vector = event->m_player_orientation * Geommath::WorldFront;
					auto trans = m_parentWorld->GetComponent<Transform3DCom>(entity);
					trans->SetLocalScale(Vec3f(0.5));
					trans->SetGlobalPos(event->m_player_position + velocity_vector * player_radius);
					trans->SetGlobalRot(event->m_player_orientation);
					trans->SetGlobalVel(velocity_vector * 10.0f);
				}
				{
					auto body = m_parentWorld->GetComponent<Body3DCom>(entity);
					body->m_bodyInfo.type = RBType::dynamicBody;
					body->m_bodyInfo.colliderDimensionExtent = 0.5;
					body->m_bodyInfo.entityTypeIngoreSet.emplace((EntityType)GameEntityType::PLAYER);
				}
			}
		}
		else
		{
			throw NotImplementedException();
		}
	}
}

void longmarch::Prototype2PlayerArsenalComSys::_ON_PROJECTILE_COLLISION(EventQueue<EngineEventType>::EventPtr e)
{
	if (m_parentWorld == GameWorld::GetCurrent())
	{
		auto event = std::static_pointer_cast<EngineCollisionEvent>(e);
		DEBUG_PRINT(Str(event->m_entity1) + " collide with " + Str(event->m_entity2));
		auto e1 = event->m_entity1;
		auto e2 = event->m_entity2;
		if (e1.GetType() == (EntityType)(GameEntityType::PROJECTILE) && e2.GetType() == (EntityType)(GameEntityType::ENEMY_ASTERIOD) ||
			e2.GetType() == (EntityType)(GameEntityType::PROJECTILE) && e1.GetType() == (EntityType)(GameEntityType::ENEMY_ASTERIOD))
		{
			auto event1 = MemoryManager::Make_shared<EngineGCEvent>(EntityDecorator{ e1, m_parentWorld });
			auto event2 = MemoryManager::Make_shared<EngineGCEvent>(EntityDecorator{ e2, m_parentWorld });
			m_GC.push_back(event1);
			m_GC.push_back(event2);
		}
	}
}

void longmarch::Prototype2PlayerArsenalComSys::_ON_GEN_SPACE_SHIP(EventQueue<Prototype2EventType>::EventPtr e)
{
	if (m_parentWorld == GameWorld::GetCurrent())
	{
		// This function shows an example of using dynamic cast to have one event callback handling multiple event types
		if (auto event = std::dynamic_pointer_cast<Prototype2PlayerGenerateSpaceShip>(e))
		{
			auto e_type = (EntityType)GameEntityType::SPACE_SHIP;
			auto entity = m_parentWorld->GenerateEntity3D(e_type, true, true);
			auto player = m_parentWorld->GetTheOnlyEntityWithType((EntityType)GameEntityType::PLAYER);
			m_spaceshipDatas.emplace_back(SpaceshipData{ .m_this = entity, .owner = player, .lifeTime = 9999.0f });
			{
				auto rm = ResourceManager<Scene3DNode>::GetInstance();
				auto meshName = "super_low_poly_spaceship_prefab";
				auto mesh = rm->TryGet(meshName)->Get()->Copy();
				auto scene = m_parentWorld->GetComponent<Scene3DCom>(entity);
				scene->SetVisiable(true);
				scene->SetSceneData(mesh);
			}
			{
				auto body_player = m_parentWorld->GetComponent<Body3DCom>(player);
				auto player_radius = body_player->m_body->GetShape()->GetRadius();
				auto velocity_vector = event->m_player_orientation * Geommath::WorldFront;
				auto trans = m_parentWorld->GetComponent<Transform3DCom>(entity);
				trans->SetLocalScale(Vec3f(.15f));
				trans->SetGlobalPos(event->m_player_position + velocity_vector * player_radius);
				trans->SetGlobalRot(Geommath::ApplyGLTF2World(event->m_player_orientation));
				trans->SetGlobalVel(velocity_vector * 2.0f);
			}
			{
				auto body = m_parentWorld->GetComponent<Body3DCom>(entity);
				body->m_bodyInfo.type = RBType::noCollision;
			}
			{
				// Adding AIControllerCom to the root entity and fetching it
				AIControllerCom aiCom(entity);
				// Setting up BT and Blackboard
				aiCom.SetBT(("$asset:BehaviorTrees/AttackAsteriod.json"));
				m_parentWorld->AddComponent<AIControllerCom>(entity.GetEntity(), aiCom);
			}
		}
		else
		{
			throw NotImplementedException();
		}
	}
}
