#pragma once
#include "engine/ecs/BaseComponentSystem.h"
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/ComponentDecorator.h"
#include "engine/ecs/GameWorld.h"
#include "../../components/AIControllerCom.h"

namespace longmarch
{
	class AIControllerComSys final : public BaseComponentSystem
	{
	public:
		NONCOPYABLE(AIControllerComSys);

		AIControllerComSys();

		virtual void PostRenderUpdate(double dt) override;
		virtual std::shared_ptr<BaseComponentSystem> Copy() const override;

	private:
		std::shared_ptr<Blackboard> m_bb{ nullptr };
	};
}