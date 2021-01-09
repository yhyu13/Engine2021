#include "application-precompiled-header.h"
#include "BehaviorTree.h"

#include "engine/ecs/header/header.h"
#include "ecs/header/header.h"
#include "events/CustomEvents.h"

void Move_to_player(EntityDecorator agent, float dt);
void Spaceship_attack_asteriod(EntityDecorator agent, float dt);

AAAAgames::BehaviorTree::BehaviorTree()
	: rootnode(), depth(0), funcname1(), funcname2(), cfnode()
{

}

void AAAAgames::BehaviorTree::RunBT(EntityDecorator agent, float dt)
{
	//TODO: while () Tree does not say end
	{
		if (rootnode == "Sequencer" || cfnode == "Sequencer")
		{
			if (funcname1 == "Move_to_Player_if_found")
			{
				Move_to_player(agent, dt);
			}
			else if (funcname1 == "Spaceship_attack_asteriod")
			{
				Spaceship_attack_asteriod(agent, dt);
			}
		}
		else if (rootnode == "Selector" || cfnode == "Selector") //cfnode part not done yet
		{
		}
		else
		{
			// Maybe rootnode not assigned, 
			// also check if update is called with the desired frame time
			// so that component's update runs
		}
	}
}

void AAAAgames::BehaviorTree::BTDeserialize(const fs::path& filepath)
{
	const auto& Filename = FileSystem::GetCachedJsonCPP(filepath);
	if (Filename.isNull())
	{
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Check File/Location and Retry");
	}
	rootnode = Filename["Test_BehaviorTree"]["root_node"].asString();
	const auto& child = Filename["Test_BehaviorTree"]["child"];
	funcname1 = child[0]["leaf_node"].asString();
}

void Move_to_player(EntityDecorator agent, float dt) // Seek player
{
	auto world = agent.GetWorld();
	auto trans = world->GetComponent<Transform3DCom>(agent);
	auto player = world->GetTheOnlyEntityWithType((EntityType)(GameEntityType::PLAYER));

	auto look_at_vec = world->GetComponent<Transform3DCom>(player)->GetGlobalPos() - trans->GetGlobalPos();
	trans->AddGlobalPos(Geommath::Normalize(look_at_vec) * dt * 4.0f);
}

void Spaceship_attack_asteriod(EntityDecorator agent, float dt)
{
	auto world = agent.GetWorld();
	auto idCom = agent.GetComponent<IDNameCom>();
	auto unique_name = idCom->GetUniqueName();
	auto aiCom = agent.GetComponent<AIControllerCom>();
	auto bb = aiCom->GetBlackboard();

	{
		// Strafing target
		auto _target_key = unique_name + "_target";
		auto entities = world->GetAllEntityWithType((EntityType)(GameEntityType::ENEMY_ASTERIOD));
		if (entities.size() == 0)
		{
			goto _ROTATE_TO_VEL;
		}
		if (!bb->has_value(_target_key))
		{
			// Simply choose the first entity
			auto target = entities[RAND_I(0, entities.size()) % entities.size()];
			bb->set_value(_target_key, target);
		}
		{
			auto target = bb->get_value<Entity>(_target_key);
			if (!A4GAMES_Contains(entities, target))
			{
				// Simply choose the first entity if older entity is expired
				auto _target = entities[RAND_I(0, entities.size()) % entities.size()];
				bb->set_value(_target_key, _target);
				target = _target;
			}
			auto target_trans = world->GetComponent<Transform3DCom>(target);
			auto target_pos = target_trans->GetGlobalPos();

			auto trans = world->GetComponent<Transform3DCom>(agent);
			auto agent_pos = trans->GetGlobalPos();

			auto look_at_vec = target_pos - agent_pos;
			if (Geommath::Length(look_at_vec) >= 5.0f)
			{
				trans->SetGlobalVel(Geommath::Normalize(look_at_vec) * 2.0f);
			}
			else
			{
				trans->SetGlobalVel(Quaternion(Geommath::RotationMat(Geommath::ROT_AXIS::YAW, PI * 0.5f)) * Geommath::Normalize(look_at_vec) * 2.0f);
			}
		}
	}
	{
		// Firing logic
		auto _target_key = unique_name + "_target";
		auto _fire_timer_key = unique_name + "_fire_timer";
		auto _fire_timer_count_key = unique_name + "_fire_timer_count";
		if (!bb->has_value(_fire_timer_key))
		{
			bb->set_value(_fire_timer_key, 2.5f);
		}
		if (!bb->has_value(_fire_timer_count_key))
		{
			bb->set_value(_fire_timer_count_key, .0f);
		}
		auto _fire_timer = bb->get_value<float>(_fire_timer_key);
		auto _fire_timer_count = bb->get_value<float>(_fire_timer_count_key);
		if (_fire_timer_count >= _fire_timer)
		{
			// Fire
			if (bb->has_value(_target_key))
			{
				auto target = bb->get_value<Entity>(_target_key);
				auto target_trans = world->GetComponent<Transform3DCom>(target);
				auto target_pos = target_trans->GetGlobalPos();

				auto trans = world->GetComponent<Transform3DCom>(agent);
				auto agent_pos = trans->GetGlobalPos();
				auto look_at_target = Geommath::FromVectorPair(Geommath::WorldFront, target_pos - agent_pos);
				{
					auto queue = EventQueue<Prototype2EventType>::GetInstance();
					auto e = MemoryManager::Make_shared<Prototype2SpaceShipGenerateProjectile>(agent, look_at_target, trans->GetGlobalPos());
					queue->Publish(e);
				}
				bb->set_value(_fire_timer_count_key, .0f);
			}
		}
		else
		{
			bb->set_value(_fire_timer_count_key, _fire_timer_count + dt);
		}
	}

	{
		_ROTATE_TO_VEL:
		auto trans = world->GetComponent<Transform3DCom>(agent);
		auto look_at_vec = trans->GetGlobalVel();
		auto look_at_target = Geommath::FromVectorPair(Geommath::WorldFront, look_at_vec);
		auto agent_rot_world = Geommath::ApplyGLTF2WorldInv(trans->GetGlobalRot());
		auto delta_rot_world = Geommath::Slerp(agent_rot_world, look_at_target, dt * 4.0f);
		trans->SetGlobalRot(Geommath::ApplyGLTF2World(delta_rot_world));
	}
}