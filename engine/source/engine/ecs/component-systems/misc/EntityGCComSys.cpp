#include "engine-precompiled-header.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"
#include "EntityGCComSys.h"
#include "engine/ecs/components/ChildrenCom.h"

void longmarch::EntityGCComSys::RemoveAllRegisteredEntities()
{
	BaseComponentSystem::RemoveAllRegisteredEntities();
	{
		LOCK_GUARD_NC();
		for (auto e = firstEntityType; e != lastEntityType; ++e)
		{
			auto& es = m_parentWorld->GetAllEntityWithType(e);
			m_UserRegisteredEntities.insert(m_UserRegisteredEntities.end(), es.begin(), es.end());
		}
	}
	GC();
}

void longmarch::EntityGCComSys::PostRenderUpdate(double dt)
{
	GC();
}

void longmarch::EntityGCComSys::GC()
{
	LOCK_GUARD_NC();
	for (auto& entity : m_UserRegisteredEntities)
	{
		DEBUG_PRINT("Engine GC : Delete " + Str(entity));
		m_parentWorld->RemoveEntity(entity);
	}
	m_UserRegisteredEntities.clear();
}

void longmarch::EntityGCComSys::_ON_GC(EventQueue<EngineEventType>::EventPtr e)
{
	LOCK_GUARD_NC();
	auto event = std::static_pointer_cast<EngineGCEvent>(e);
	if (event->m_entity.Valid() && m_parentWorld == event->m_entity.GetWorld())
	{
		m_UserRegisteredEntities.push_back(event->m_entity);
		m_parentWorld->InactivateHelper(event->m_entity);
		m_parentWorld->RemoveFromParentHelper(event->m_entity);
	}
}

void longmarch::EntityGCComSys::_ON_GC_RECURSIVE(EventQueue<EngineEventType>::EventPtr e)
{
	auto event = std::static_pointer_cast<EngineGCEvent>(e);
	if (event->m_entity.Valid() && m_parentWorld == event->m_entity.GetWorld())
	{
		m_parentWorld->RemoveFromParentHelper(event->m_entity);
		GCRecursive(event->m_entity);
	}
}

void longmarch::EntityGCComSys::GCRecursive(EntityDecorator e)
{
	this->LockNC();
	m_UserRegisteredEntities.push_back(e);
	m_parentWorld->InactivateHelper(e);
	this->UnlockNC();
	for (auto& child : m_parentWorld->GetComponent<ChildrenCom>(e)->GetChildren())
	{
		GCRecursive(EntityDecorator{ child ,e.GetWorld() });
	}
}
