#pragma once
#include "Entity.h"
#include "engine/core/exception/EngineException.h"
#include "engine/core/thread/Lock.h"
#include "engine/core/utility/TypeHelper.h"

namespace longmarch
{
	/**
	 * @brief Entity managers takes care of assigning entity-ids to entities during their creations.
	 *
	 *  It makes sure that no two entities within
	 *	the system share the same entity-id.
	 *
	 * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (yohan680919@gmail.com)
	 */
	class EntityManager : BaseAtomicClassNC
	{
	public:
		NONCOPYABLE(EntityManager);
		EntityManager() = default;

		inline const Entity Create(EntityType type)
		{
			LOCK_GUARD_NC();
			m_typeToEntity[type].push_back(++m_entityID);
			return Entity(m_entityID, type);
		}

		//! Simply remove the entity from the entity manager, if you want to remove all components, consult GameWorld
		inline void Destroy(const Entity& entity)
		{
			LOCK_GUARD_NC();
			std::vector<EntityID>& vec = m_typeToEntity[entity.m_type];
			vec.erase(std::remove(vec.begin(), vec.end(), entity.m_id), vec.end());
		}

		inline const std::vector<EntityID> GetAllEntityIDWithType(EntityType type) const
		{
			LOCK_GUARD_NC();
			if (auto it = m_typeToEntity.find(type); it != m_typeToEntity.end())
			{
				return it->second;
			}
			else
			{
				return std::vector<EntityID>();
			}
		}

		inline const Entity GetEntityFromID(EntityID ID) const
		{
			LOCK_GUARD_NC();
			for (const auto& [type, ids] : m_typeToEntity)
			{
				for (const auto& id : ids)
				{
					if (ID == id)
					{
						return Entity(id, type);
					}
				}
			}
			return Entity();
		}

		inline std::shared_ptr<EntityManager> Copy() const
		{
			LOCK_GUARD_NC();
			auto ret = MemoryManager::Make_shared<EntityManager>();
			ret->m_typeToEntity = m_typeToEntity;
			ret->m_entityID = m_entityID;
			return ret;
		}

	private:
		LongMarch_UnorderedMap_Par_node<EntityType, std::vector<EntityID>> m_typeToEntity;
		EntityID m_entityID{};
	};
}
