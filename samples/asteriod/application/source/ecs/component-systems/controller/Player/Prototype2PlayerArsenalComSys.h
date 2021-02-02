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

#pragma once
#include "engine/ecs/BaseComponentSystem.h"
#include "engine/math/Geommath.h"
#include "events/EventType.h"

namespace longmarch
{
	struct SpaceshipData
	{
		Entity m_this;
		Entity owner;
		float lifeTime{ 2.0f };
	};

	struct ProjectileData
	{
		Entity m_this;
		Entity owner;
		float lifeTime{2.0f};
	};

	class Prototype2PlayerArsenalComSys final : public BaseComponentSystem
	{
	public:
		NONCOPYABLE(Prototype2PlayerArsenalComSys);
		COMSYS_DEFAULT_COPY(Prototype2PlayerArsenalComSys);

		Prototype2PlayerArsenalComSys() = default;
		virtual void Init() override;
		virtual void Update(double dt) override;
		virtual void PostRenderUpdate(double dt) override;

	private:

		void _ON_GEN_PROJECTILE(EventQueue<Prototype2EventType>::EventPtr e);
		void _ON_PROJECTILE_COLLISION(EventQueue<EngineEventType>::EventPtr e);

		void _ON_GEN_SPACE_SHIP(EventQueue<Prototype2EventType>::EventPtr e);

	public:
		int m_MaxProjectileFireRate{ 2 };
		int m_MaxNumProjectile{ 1 };
		int m_MaxNumDrone{ 1 };

	private:
		LongMarch_Vector<SpaceshipData> m_spaceshipDatas;
		LongMarch_Vector<ProjectileData> m_projectileDatas;
		LongMarch_Vector<std::shared_ptr<EngineGCEvent>> m_GC;

		float m_projectileFireDeltaTime{ 0.f };
	};
}