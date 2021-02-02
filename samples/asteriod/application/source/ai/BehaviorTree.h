
#pragma once
#include "engine/ecs/components/3d/Transform3DCom.h"
#include "engine/core/file-system/FileSystem.h"
#include "Blackboard.h"
#include "engine/ecs/components/3d/Body3DCom.h"

namespace longmarch
{
	class BehaviorTree
	{
	private:
		std::string rootnode;
		int depth;
		std::string funcname1;
		std::string funcname2;
		std::string cfnode; // Control flow node

		/*
		TODO: for later, create a struct that takes in node values
		with cfnode, decoratornode, leafnode and depth and child to to know where depth increases.
		Recursively check for children in the tree and enter depth to call each of the node functions 
		with their respective call order from cfnode.
		*/

		//Blackboard bb; // Testing functionality of blackboard

	public:
		BehaviorTree();
		void BTDeserialize(const fs::path& filename);
		void RunBT(EntityDecorator agent, float dt);
	};

}