#include "engine-precompiled-header.h"
#include "ChildrenCom.h"
#include "ParentCom.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/components/3d/Transform3DCom.h"

AAAAgames::ChildrenCom::ChildrenCom(const EntityDecorator& _this)
	:
	BaseComponent(_this.GetWorld()),
	m_this(_this.GetEntity())
{
}

void AAAAgames::ChildrenCom::AddEntity(const Entity& child)
{
	LOCK_GUARD2();
	if (!A4GAMES_Contains(m_children, child))
	{
		m_children.emplace_back(child);

		auto world = m_world;
		world->GetComponent<ParentCom>(child)->SetParentWOR(m_this);
	}
}

bool AAAAgames::ChildrenCom::HasEntity(const Entity& child)
{
	LOCK_GUARD2();
	return A4GAMES_Contains(m_children, child);
}

void AAAAgames::ChildrenCom::AddEntityWOR(const Entity& child)
{
	LOCK_GUARD2();
	if (!A4GAMES_Contains(m_children, child))
	{
		m_children.emplace_back(child);
	}
}

const A4GAMES_Vector<Entity> AAAAgames::ChildrenCom::GetChildren() {
	LOCK_GUARD2();
	return m_children;
}

bool AAAAgames::ChildrenCom::RemoveEntity(const Entity& child)
{
	LOCK_GUARD2();
	if (auto it = std::find(m_children.begin(), m_children.end(), child); it != m_children.end())
	{
		if (auto transCom = m_world->GetComponent<Transform3DCom>(*it); transCom.Valid())
		{
			transCom->ResetParentModelTr();
		}
		m_children.erase(it);
		return true;
	}
	else
	{
		return false;
	}
}

void AAAAgames::ChildrenCom::RemoveAll()
{
	LOCK_GUARD2();
	m_children.clear();
}

bool AAAAgames::ChildrenCom::IsLeaf()
{
	LOCK_GUARD2();
	return m_children.empty();
}