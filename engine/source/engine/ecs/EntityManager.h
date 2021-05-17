#pragma once
#include "engine/EngineEssential.h"
#include "Entity.h"

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
			auto& ids = m_typeToEntity[type];
			ids.push_back(++m_entityID);
			return Entity(m_entityID, type);
		}

		// Should use the same poping strategy as component manager to counter memory diffusion
		inline void Destroy(const Entity& entity)
		{
			LOCK_GUARD_NC();
			auto& vec = m_typeToEntity[entity.m_type];
			if (auto index = LongMarch_findFristIndex(vec, entity.m_id); 
				index != -1)
			{
				vec.erase(vec.begin() + index);
			}
		}

		inline const LongMarch_Vector<EntityID> GetAllEntityIDWithType(EntityType type) const
		{
			LOCK_GUARD_NC();
			if (auto it = m_typeToEntity.find(type); it != m_typeToEntity.end())
			{
				return it->second;
			}
			else
			{
				return LongMarch_Vector<EntityID>();
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
		LongMarch_UnorderedMap_Par_node<EntityType, LongMarch_Vector<EntityID>> m_typeToEntity;
		EntityID m_entityID{};
	};
}
