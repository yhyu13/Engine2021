#include "application-precompiled-header.h"
#include "BehaviorTree.h"

#include "engine/ecs/header/header.h"
#include "ecs/header/header.h"
#include "events/CustomEvents.h"

void Move_to_player(const EntityDecorator& agent, float dt);

longmarch::BehaviorTree::BehaviorTree()
	: rootnode(), depth(0), funcname1(), funcname2(), cfnode()
{

}

void longmarch::BehaviorTree::RunBT(const EntityDecorator& agent, float dt)
{
	//TODO: while () Tree does not say end
	{
		if (rootnode == "Sequencer" || cfnode == "Sequencer")
		{
			if (funcname1 == "Move_to_Player_if_found")
			{
				Move_to_player(agent, dt);
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

void longmarch::BehaviorTree::BTDeserialize(const fs::path& filepath)
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

void Move_to_player(const EntityDecorator& agent, float dt) // Seek player
{
	auto world = agent.GetWorld();
	auto trans = world->GetComponent<Transform3DCom>(agent);
	auto player = world->GetTheOnlyEntityWithType((EntityType)(GameEntityType::PLAYER));

	auto look_at_vec = world->GetComponent<Transform3DCom>(player)->GetGlobalPos() - trans->GetGlobalPos();
	trans->AddGlobalPos(Geommath::Normalize(look_at_vec) * dt * 4.0f);
}