#pragma once
#include "engine/ecs/BaseComponentSystem.h"
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/ComponentDecorator.h"
#include "engine/ecs/GameWorld.h"

namespace longmarch
{
	class Prototype2EnemyGeneratorSystem final : public BaseComponentSystem
	{
	public:
		NONCOPYABLE(Prototype2EnemyGeneratorSystem);
		COMSYS_DEFAULT_COPY(Prototype2EnemyGeneratorSystem);

		Prototype2EnemyGeneratorSystem() = default;
		virtual void PostRenderUpdate(double dt) override;
		
	private:
		Timer timer; 
		int count;
	};
}