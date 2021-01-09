#pragma once
#include "engine/ecs/BaseComponentSystem.h"
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/ComponentDecorator.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/components/3d/Transform3DCom.h"
#include "engine/ecs/components/3d/Body3DCom.h"
#include "engine/physics/PhysicsManager.h"
#include "engine/events/engineEvents/EngineEventType.h"

namespace longmarch
{
	class Body3DComSys final : public BaseComponentSystem
	{
	public:
		NONCOPYABLE(Body3DComSys);

		Body3DComSys();
		~Body3DComSys();

		virtual void Init() override;
		virtual void PreRenderUpdate(double dt) override;
		virtual void Update(double dt) override;
		virtual std::shared_ptr<BaseComponentSystem> Copy() const override;

		virtual std::shared_ptr<Scene> GetScene() const 
		{ 
			return m_scene; 
		}

	private:
		void _ON_GC(EventQueue<EngineEventType>::EventPtr e);
		void _ON_GC_RECURSIVE(EventQueue<EngineEventType>::EventPtr e);
		void GCRecursive(EntityDecorator e);

	private:
		std::shared_ptr<Scene> m_scene;
	};
}