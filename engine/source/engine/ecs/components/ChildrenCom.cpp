#include "engine-precompiled-header.h"
#include "ChildrenCom.h"
#include "ParentCom.h"
#include "engine/ecs/GameWorld.h"
#include "engine/ecs/components/3d/Transform3DCom.h"

longmarch::ChildrenCom::ChildrenCom(const EntityDecorator& _this)
	:
	BaseComponent(_this.GetVolatileWorld()),
	m_this(_this.GetEntity())
{
}

void longmarch::ChildrenCom::AddEntity(const Entity& child)
{
	LOCK_GUARD2();
	if (!LongMarch_Contains(m_children, child))
	{
		m_children.emplace_back(child);

		auto world = m_world;
		world->GetComponent<ParentCom>(child)->SetParentWOR(m_this);
	}
}

bool longmarch::ChildrenCom::HasEntity(const Entity& child)
{
	LOCK_GUARD2();
	return LongMarch_Contains(m_children, child);
}

void longmarch::ChildrenCom::AddEntityWOR(const Entity& child)
{
	LOCK_GUARD2();
	if (!LongMarch_Contains(m_children, child))
	{
		m_children.emplace_back(child);
	}
}

const LongMarch_Vector<Entity> longmarch::ChildrenCom::GetChildren() {
	LOCK_GUARD2();
	return m_children;
}

bool longmarch::ChildrenCom::RemoveEntity(const Entity& child)
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

void longmarch::ChildrenCom::RemoveAll()
{
	LOCK_GUARD2();
	m_children.clear();
}

bool longmarch::ChildrenCom::IsLeaf()
{
	LOCK_GUARD2();
	return m_children.empty();
}