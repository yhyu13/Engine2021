#pragma once
#include "engine/ecs/BaseComponentSystem.h"
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/ComponentDecorator.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/EntityType.h"
#include "engine/core/utility/TypeHelper.h"

// yuhang : since EntityGCComSys handles both engine and application entity types, and since we don't really know how many of them are there
// so we simply set a pesudo range
#define FIRST_ENTITY_TYPE_ENUM 0
#define LAST_ENTITY_TYPE_ENUM 256

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
		constexpr static EntityType firstEntityType = { FIRST_ENTITY_TYPE_ENUM };
		constexpr static EntityType lastEntityType = { LAST_ENTITY_TYPE_ENUM };

	public:
		NONCOPYABLE(EntityGCComSys);
		COMSYS_DEFAULT_COPY(EntityGCComSys);

		EntityGCComSys()
		{
			m_UserRegisteredEntities.reserve(1024);
		}

		virtual void Init() override
		{
			auto queue = EventQueue<EngineEventType>::GetInstance();
			ManageEventSubHandle(queue->Subscribe<EntityGCComSys>(this, EngineEventType::GC, &EntityGCComSys::_ON_GC));
			ManageEventSubHandle(queue->Subscribe<EntityGCComSys>(this, EngineEventType::GC_RECURSIVE, &EntityGCComSys::_ON_GC_RECURSIVE));
		}

		virtual void RemoveAllRegisteredEntities() override;
		virtual void PostRenderUpdate(double dt) override;

	private:
		void GC();

		void _ON_GC(EventQueue<EngineEventType>::EventPtr e);
		void _ON_GC_RECURSIVE(EventQueue<EngineEventType>::EventPtr e);
		void GCRecursive(EntityDecorator e);
	};
}

#undef FIRST_ENTITY_TYPE_ENUM
#undef LAST_ENTITY_TYPE_ENUM
