#include "engine-precompiled-header.h"
#include "ParentCom.h"
#include "ChildrenCom.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/EntityType.h"

longmarch::ParentCom::ParentCom(const EntityDecorator& _this)
	:
	BaseComponent(_this.Volatile().GetWorld()),
	m_this(_this.GetEntity()),
	m_Parent(Entity())
{
}

void longmarch::ParentCom::SetParent(const Entity& parent)
{
	LOCK_GUARD();
	auto world = m_world;
	{
		// Remove previous parent
		if (m_Parent != Entity())
		{
			world->GetComponent<ChildrenCom>(parent)->RemoveEntity(m_this);
		}
		m_Parent = parent;
	}
	world->GetComponent<ChildrenCom>(parent)->AddEntityWORecursion(m_this);
}

void longmarch::ParentCom::SetParentWORecursion(const Entity& parent)
{
	LOCK_GUARD();
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
	LOCK_GUARD();
	return m_Parent;
}

bool longmarch::ParentCom::IsRoot()
{
	LOCK_GUARD();
	return m_Parent == Entity();
}

bool longmarch::ParentCom::IsSceneRoot()
{
	LOCK_GUARD();
	return m_Parent.m_type == (EntityType)EngineEntityType::SCENE_ROOT;
}
