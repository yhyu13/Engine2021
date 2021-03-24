#pragma once
#include "engine/ecs/BaseComponentSystem.h"
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/ComponentDecorator.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/EntityType.h"
#include "engine/core/utility/TypeHelper.h"

namespace longmarch
{
	/**
	 * @brief The Garbage collection component system for GC events
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
	 */
	class EntityGCComSys final : public BaseComponentSystem
	{
	public:
		EntityType firstEntityType = { (EntityType)0 };//{ (EntityType)EngineObjectType::EMPTY };
		EntityType lastEntityType = { (EntityType)256 };//{ (EntityType)EngineObjectType::NUM };

	public:
		NONCOPYABLE(EntityGCComSys);
		COMSYS_DEFAULT_COPY(EntityGCComSys);

		EntityGCComSys()
		{
			m_GCList.reserve(1024);
		}

		virtual void Init() override
		{
			auto queue = EventQueue<EngineEventType>::GetInstance();
			ManageEventSubHandle(queue->Subscribe<EntityGCComSys>(this, EngineEventType::GC, &EntityGCComSys::_ON_GC));
			ManageEventSubHandle(queue->Subscribe<EntityGCComSys>(this, EngineEventType::GC_RECURSIVE, &EntityGCComSys::_ON_GC_RECURSIVE));
		}

		virtual void RemoveAllEntities() override;
		virtual void PostRenderUpdate(double dt) override;

	private:
		void GC();

		void _ON_GC(EventQueue<EngineEventType>::EventPtr e);
		void _ON_GC_RECURSIVE(EventQueue<EngineEventType>::EventPtr e);
		void GCRecursive(EntityDecorator e);

	private:
		LongMarch_Vector<Entity> m_GCList;
	};
}