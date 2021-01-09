#pragma once
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/components/3d/Transform3DCom.h"
#include "../../ai/BehaviorTree.h"
#include "../../ai/Blackboard.h"

namespace AAAAgames
{
	struct CACHE_ALIGN32 AIControllerCom final : BaseComponent<AIControllerCom>
	{
	public:
		// Member Functions
		AIControllerCom() = delete;
		explicit AIControllerCom(Entity _this);

		void Update(double dt);

		BehaviorTree& GetBT();
		void SetBT(const fs::path& filepath);

		Blackboard* GetBlackboard();
		void SetBlackboard(Blackboard* blackboard);

	public:
		Entity m_this;

	private:
		BehaviorTree BT;
		Blackboard* bb{ nullptr };
	};
}


