#pragma once
#include "engine/ecs/BaseComponent.h"
#include "engine/ecs/EntityDecorator.h"

namespace longmarch
{
	struct ChildrenCom;
	/*
	Data class that stores references to owner entity type
	*/
	struct CACHE_ALIGN16 ParentCom final: BaseComponent<ParentCom>
	{
		explicit ParentCom(const EntityDecorator & _this);
		void SetParent(const Entity& parent);
		const Entity GetParent();
		bool IsRoot();
		bool IsSceneRoot();

		friend ChildrenCom;
	private:
		void SetParentWOR(const Entity& parent); //!< Set parent without recursion
	private:
		Entity m_this;
		Entity m_Parent;
	};
}
