#pragma once

#include "engine/ecs/BaseComponentSystem.h"
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/ComponentDecorator.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/components/3d/Particle3DCom.h"
#include "engine/ecs/components/PerspectiveCameraCom.h"
#include "engine/Engine.h"

namespace longmarch
{
	class Particle3DComSys final : public BaseComponentSystem
	{
	public:
		NONCOPYABLE(Particle3DComSys);
		COMSYS_DEFAULT_COPY(Particle3DComSys);

		Particle3DComSys();
		virtual void Update(double dt) override;
	};
}
