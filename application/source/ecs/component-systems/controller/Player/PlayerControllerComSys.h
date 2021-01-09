#pragma once
#include "engine/ecs/BaseComponentSystem.h"

namespace AAAAgames
{
	class PlayerControllerComSys final : public BaseComponentSystem
	{
	public:
		NONCOPYABLE(PlayerControllerComSys);
		COMSYS_DEFAULT_COPY(PlayerControllerComSys);

		PlayerControllerComSys() = default;
		virtual void Update(double dt) override;
	};
}