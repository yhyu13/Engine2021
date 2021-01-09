#include "engine-precompiled-header.h"
#include "ParentCom.h"
#include "ChildrenCom.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/EntityType.h"

AAAAgames::ParentCom::ParentCom(const EntityDecorator& _this)
	:
	BaseComponent(_this.GetWorld()),
	m_this(_this.GetEntity()),
	m_Parent(Entity())
{
}

void AAAAgames::ParentCom::SetParent(const Entity& parent)
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

void AAAAgames::ParentCom::SetParentWOR(const Entity& parent)
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

const Entity AAAAgames::ParentCom::GetParent() {
	LOCK_GUARD2();
	return m_Parent;
}

bool AAAAgames::ParentCom::IsRoot()
{
	LOCK_GUARD2();
	return m_Parent == Entity();
}

bool AAAAgames::ParentCom::IsSceneRoot()
{
	LOCK_GUARD2();
	return m_Parent.m_type == (EntityType)EngineEntityType::SCENE_ROOT;
}