#include "engine-precompiled-header.h"
#include "ParentCom.h"
#include "ChildrenCom.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/EntityType.h"

longmarch::ParentCom::ParentCom(const EntityDecorator& _this)
	:
	BaseComponent(_this.GetVolatileWorld()),
	m_this(_this.GetEntity()),
	m_Parent(Entity())
{
}

void longmarch::ParentCom::SetParent(const Entity& parent)
{
	LOCK_GUARD2();
	auto world = m_world;
	{
		// Remove previous parent
		if (m_Parent != Entity())
		{
			world->GetComponent<ChildrenCom>(parent)->RemoveEntity(m_this);
		}
		m_Parent = parent;
	}
	world->GetComponent<ChildrenCom>(parent)->AddEntityWOR(m_this);
}

void longmarch::ParentCom::SetParentWOR(const Entity& parent)
{
	LOCK_GUARD2();
	auto world = m_world;
	{
		// Remove previous parent
		if (m_Parent != Entity())
		{
			world->GetComponent<ChildrenCom>(parent)->RemoveEntity(m_this);
		}
		m_Parent = parent;
	}
}

const Entity longmarch::ParentCom::GetParent() {
	LOCK_GUARD2();
	return m_Parent;
}

bool longmarch::ParentCom::IsRoot()
{
	LOCK_GUARD2();
	return m_Parent == Entity();
}

bool longmarch::ParentCom::IsSceneRoot()
{
	LOCK_GUARD2();
	return m_Parent.m_type == (EntityType)EngineEntityType::SCENE_ROOT;
}